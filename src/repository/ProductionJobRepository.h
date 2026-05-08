#pragma once
#include "model/ProductionJob.h"
#include <vector>
#include <optional>
#include <string>

class ProductionJobRepository {
public:
    explicit ProductionJobRepository(const std::string& filePath = "data/production_jobs.json");

    void add(const ProductionJob& job);
    std::vector<ProductionJob> findAll() const;
    std::optional<ProductionJob> findByOrderId(const std::string& orderId) const;
    void update(const ProductionJob& job);
    void save() const;

private:
    std::string                        m_filePath;
    mutable std::vector<ProductionJob> m_jobs;
    mutable bool                       m_dirty = false;

    void reload() const;
    void ensureFresh() const;
};
