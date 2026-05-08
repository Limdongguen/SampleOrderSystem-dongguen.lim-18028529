#pragma once
#include "repository/ProductionJobRepository.h"
#include "model/Order.h"
#include "model/Sample.h"
#include "model/ProductionJob.h"
#include <queue>
#include <vector>
#include <string>
#include <memory>

class ProductionService {
public:
    explicit ProductionService(
        const std::string& productionFilePath = "data/production_jobs.json"
    );

    bool                       enqueue(const Order& order, const Sample& sample);
    std::vector<ProductionJob> getQueue() const;

private:
    std::unique_ptr<ProductionJobRepository> m_jobRepo;
    std::queue<ProductionJob>                m_queue;

    static int    calcShortage(const Order& order, const Sample& sample);
    static int    calcActualProduction(int shortage, double yield);
    static double calcTotalTime(double avgProdTime, int actualProduction);
    static std::string generateJobId(const std::string& orderId);
};
