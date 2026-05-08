#pragma once
#include "repository/ProductionJobRepository.h"
#include "repository/SampleRepository.h"
#include "repository/OrderRepository.h"
#include "model/Order.h"
#include "model/Sample.h"
#include "model/ProductionJob.h"
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <optional>

class ProductionService {
public:
    explicit ProductionService(
        const std::string& productionFilePath = "data/production_jobs.json",
        const std::string& sampleFilePath     = "data/samples.json",
        const std::string& orderFilePath      = "data/orders.json"
    );

    bool                            enqueue(const Order& order, const Sample& sample);
    void                            tickCheck();
    std::optional<ProductionJob>    getCurrentJob() const;
    std::vector<ProductionJob>      getWaitingJobs() const;
    std::vector<ProductionJob>      getQueue() const;

private:
    std::unique_ptr<ProductionJobRepository> m_jobRepo;
    std::unique_ptr<SampleRepository>        m_sampleRepo;
    std::unique_ptr<OrderRepository>         m_orderRepo;
    std::queue<ProductionJob>                m_queue;

    void startJob(ProductionJob& job);
    void completeJob(ProductionJob& job);

    static int    calcShortage(const Order& order, const Sample& sample);
    static int    calcActualProduction(int shortage, double yield);
    static double calcTotalTime(double avgProdTime, int actualProduction);
    static std::string generateJobId(const std::string& orderId);
};
