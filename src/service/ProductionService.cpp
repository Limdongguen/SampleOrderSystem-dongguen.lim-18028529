#include "service/ProductionService.h"
#include "util/TimeUtil.h"
#include <cmath>
#include <algorithm>

namespace {
    constexpr double kYieldBuffer = 0.9;
    constexpr int    kShortageMin = 1;
    constexpr const char* kJobIdPrefix = "JOB-";
}

ProductionService::ProductionService(
    const std::string& productionFilePath,
    const std::string& sampleFilePath,
    const std::string& orderFilePath
)
    : m_jobRepo   (std::make_unique<ProductionJobRepository>(productionFilePath))
    , m_sampleRepo(std::make_unique<SampleRepository>(sampleFilePath))
    , m_orderRepo (std::make_unique<OrderRepository>(orderFilePath))
{
    // WAITING/RUNNING 상태 job을 앱 시작 시 큐에 초기화
    for (const auto& job : m_jobRepo->findAll()) {
        if (job.status == JobStatus::WAITING || job.status == JobStatus::RUNNING) {
            m_queue.push(job);
        }
    }
}

bool ProductionService::enqueue(const Order& order, const Sample& sample) {
    int shortage = calcShortage(order, sample);
    if (shortage < kShortageMin) return false;

    int    actualProduction = calcActualProduction(shortage, sample.yield);
    double totalTime        = calcTotalTime(sample.avgProdTime, actualProduction);

    ProductionJob job;
    job.jobId            = generateJobId(order.orderId);
    job.orderId          = order.orderId;
    job.sampleId         = order.sampleId;
    job.shortage         = shortage;
    job.actualProduction = actualProduction;
    job.totalTime        = totalTime;
    job.status           = JobStatus::WAITING;
    job.estimatedEndTime = "";

    bool queueWasEmpty = m_queue.empty();
    m_queue.push(job);
    m_jobRepo->add(job);
    m_jobRepo->save();

    if (queueWasEmpty) {
        startJob(m_queue.front());
    }
    return true;
}

void ProductionService::startJob(ProductionJob& job) {
    job.estimatedEndTime = TimeUtil::addMinutes(TimeUtil::nowString(), job.totalTime);
    job.status           = JobStatus::RUNNING;
    m_jobRepo->update(job);
    m_jobRepo->save();
    // 큐의 front도 업데이트
    if (!m_queue.empty() && m_queue.front().jobId == job.jobId) {
        m_queue.front() = job;
    }
}

bool ProductionService::tickCheck() {
    auto allJobs = m_jobRepo->findAll();
    ProductionJob* runningJob = nullptr;
    for (auto& j : allJobs) {
        if (j.status == JobStatus::RUNNING) {
            runningJob = &j;
            break;
        }
    }
    if (runningJob == nullptr) return false;
    if (!TimeUtil::isPast(runningJob->estimatedEndTime)) return false;
    completeJob(*runningJob);
    return true;
}

void ProductionService::applySampleStockIncrease(ProductionJob& job) {
    auto sampleOpt = m_sampleRepo->findById(job.sampleId);
    if (!sampleOpt.has_value()) return;
    Sample sample = sampleOpt.value();
    sample.stock += job.actualProduction;
    m_sampleRepo->update(sample);
    m_sampleRepo->save();
}

void ProductionService::applyOrderConfirmed(ProductionJob& job) {
    auto orderOpt = m_orderRepo->findById(job.orderId);
    if (!orderOpt.has_value()) return;
    Order order     = orderOpt.value();
    order.status    = OrderStatus::CONFIRMED;
    order.updatedAt = TimeUtil::nowString();
    m_orderRepo->update(order);
    m_orderRepo->save();
}

void ProductionService::completeJob(ProductionJob& job) {
    applySampleStockIncrease(job);
    applyOrderConfirmed(job);

    job.status = JobStatus::DONE;
    m_jobRepo->update(job);
    m_jobRepo->save();

    if (!m_queue.empty()) m_queue.pop();

    if (!m_queue.empty() && m_queue.front().status == JobStatus::WAITING) {
        startJob(m_queue.front());
    }
}

std::optional<ProductionJob> ProductionService::getCurrentJob() const {
    auto allJobs = m_jobRepo->findAll();
    for (const auto& j : allJobs) {
        if (j.status == JobStatus::RUNNING) return j;
    }
    return std::nullopt;
}

std::vector<ProductionJob> ProductionService::getWaitingJobs() const {
    std::vector<ProductionJob> result;
    auto allJobs = m_jobRepo->findAll();
    for (const auto& j : allJobs) {
        if (j.status == JobStatus::WAITING) result.push_back(j);
    }
    return result;
}

std::vector<ProductionJob> ProductionService::getQueue() const {
    std::vector<ProductionJob> result;
    std::queue<ProductionJob>  copy = m_queue;
    while (!copy.empty()) {
        result.push_back(copy.front());
        copy.pop();
    }
    return result;
}

int ProductionService::calcShortage(const Order& order, const Sample& sample) {
    return order.quantity - sample.stock;
}

int ProductionService::calcActualProduction(int shortage, double yield) {
    return (int)std::ceil(shortage / (yield * kYieldBuffer));
}

double ProductionService::calcTotalTime(double avgProdTime, int actualProduction) {
    return avgProdTime * actualProduction;
}

std::string ProductionService::generateJobId(const std::string& orderId) {
    return kJobIdPrefix + orderId;
}
