#pragma once
#include "model/Sample.h"
#include "model/Order.h"
#include "model/ProductionJob.h"
#include "model/Enums.h"
#include <string>
#include <vector>

// 더미 데이터 생성 유틸리티
// data/*.json 파일에 시료 10종, 주문 20건, 생산작업 3건을 생성한다.
class DummyGen {
public:
    explicit DummyGen(
        const std::string& samplesPath    = "data/samples.json",
        const std::string& ordersPath     = "data/orders.json",
        const std::string& productionPath = "data/production_jobs.json"
    );

    // 더미 데이터를 생성하고 JSON 파일에 저장한다.
    // 성공 시 true 반환
    bool generate();

private:
    std::string m_samplesPath;
    std::string m_ordersPath;
    std::string m_productionPath;

    void generateSamples();
    void generateOrders();
    void generateProductionJobs();
    void printSummary() const;

    static std::string makeSampleId(int index);
    static std::string makeOrderId(int seq);
    static Order       buildOrder(
        const std::string& sampleId, const std::string& customerName,
        int quantity, OrderStatus status, const std::string& orderId,
        const std::string& timestamp);
    static ProductionJob buildJob(
        const std::string& orderId, const std::string& sampleId,
        int shortage, double yield, double avgProdTime,
        JobStatus status, const std::string& estimatedEndTime);
};
