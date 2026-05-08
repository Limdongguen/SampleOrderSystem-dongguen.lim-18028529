#include "repository/ProductionJobRepository.h"
#include "util/JsonFileManager.h"
#include <algorithm>

ProductionJobRepository::ProductionJobRepository(const std::string& filePath)
    : m_filePath(filePath)
{
    auto json = JsonFileManager::load(m_filePath);
    for (const auto& item : json) {
        m_jobs.push_back(item.get<ProductionJob>());
    }
}

void ProductionJobRepository::add(const ProductionJob& job) {
    m_jobs.push_back(job);
}

std::vector<ProductionJob> ProductionJobRepository::findAll() const {
    return m_jobs;
}

std::optional<ProductionJob> ProductionJobRepository::findByOrderId(const std::string& orderId) const {
    for (const auto& j : m_jobs) {
        if (j.orderId == orderId) {
            return j;
        }
    }
    return std::nullopt;
}

void ProductionJobRepository::update(const ProductionJob& job) {
    for (auto& j : m_jobs) {
        if (j.jobId == job.jobId) {
            j = job;
            return;
        }
    }
}

void ProductionJobRepository::save() const {
    nlohmann::json jsonArr = nlohmann::json::array();
    for (const auto& j : m_jobs) {
        nlohmann::json jj;
        to_json(jj, j);
        jsonArr.push_back(std::move(jj));
    }
    JsonFileManager::save(m_filePath, jsonArr);
}
