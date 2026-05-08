#include "service/ProductionService.h"
#include <cmath>

namespace {
    constexpr double kYieldBuffer   = 0.9;
    constexpr int    kShortageMin   = 1;
    const     char*  kJobIdPrefix   = "JOB-";
}

ProductionService::ProductionService(const std::string& productionFilePath)
    : m_jobRepo(std::make_unique<ProductionJobRepository>(productionFilePath))
{
    // WAITING 상태 job을 앱 시작 시 큐에 초기화
    for (const auto& job : m_jobRepo->findAll()) {
        if (job.status == JobStatus::WAITING) {
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

    m_queue.push(job);
    m_jobRepo->add(job);
    m_jobRepo->save();
    return true;
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
