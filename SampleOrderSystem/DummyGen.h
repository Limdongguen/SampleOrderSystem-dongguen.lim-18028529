#pragma once
#include <string>

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
};
