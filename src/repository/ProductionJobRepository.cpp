#include "repository/ProductionJobRepository.h"
#include "util/JsonFileManager.h"

ProductionJobRepository::ProductionJobRepository(const std::string& filePath)
    : m_filePath(filePath), m_dirty(false)
{
    reload();
}

void ProductionJobRepository::reload() const {
    m_jobs.clear();
    auto json = JsonFileManager::load(m_filePath);
    for (const auto& item : json) m_jobs.push_back(item.get<ProductionJob>());
    m_dirty = false;
}

void ProductionJobRepository::ensureFresh() const {
    if (!m_dirty) reload();
}

void ProductionJobRepository::add(const ProductionJob& job) {
    ensureFresh();
    m_jobs.push_back(job);
    m_dirty = true;
}

std::vector<ProductionJob> ProductionJobRepository::findAll() const {
    ensureFresh();
    return m_jobs;
}

std::optional<ProductionJob> ProductionJobRepository::findByOrderId(const std::string& orderId) const {
    ensureFresh();
    for (const auto& j : m_jobs) {
        if (j.orderId == orderId) return j;
    }
    return std::nullopt;
}

void ProductionJobRepository::update(const ProductionJob& job) {
    ensureFresh();
    for (auto& j : m_jobs) {
        if (j.jobId == job.jobId) { j = job; m_dirty = true; return; }
    }
}

void ProductionJobRepository::save() const {
    nlohmann::json jsonArr = nlohmann::json::array();
    for (const auto& j : m_jobs) {
        nlohmann::json jj; to_json(jj, j); jsonArr.push_back(std::move(jj));
    }
    JsonFileManager::save(m_filePath, jsonArr);
    m_dirty = false;
}
