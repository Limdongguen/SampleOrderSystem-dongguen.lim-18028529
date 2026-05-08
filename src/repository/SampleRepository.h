#pragma once
#include "model/Sample.h"
#include <vector>
#include <optional>
#include <string>

class SampleRepository {
public:
    explicit SampleRepository(const std::string& filePath = "data/samples.json");

    void add(const Sample& sample);
    std::vector<Sample> findAll() const;
    std::optional<Sample> findById(const std::string& id) const;
    std::vector<Sample> findByName(const std::string& keyword) const;
    void save() const;

private:
    std::string m_filePath;
    std::vector<Sample> m_samples;
};
