#pragma once
#include "repository/SampleRepository.h"
#include "model/Sample.h"
#include <vector>
#include <optional>
#include <string>
#include <memory>

class SampleService {
public:
    explicit SampleService(const std::string& filePath = "data/samples.json");

    bool registerSample(const std::string& name, double avgProdTime, double yield);
    std::vector<Sample> getAllSamples() const;
    std::vector<Sample> searchByName(const std::string& keyword) const;
    std::optional<Sample> getSampleById(const std::string& id) const;

private:
    std::unique_ptr<SampleRepository> m_repo;

    bool isValidInput(const std::string& name, double avgProdTime, double yield) const;
    bool isDuplicateName(const std::string& name) const;
};
