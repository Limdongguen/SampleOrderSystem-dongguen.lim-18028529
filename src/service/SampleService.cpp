#include "service/SampleService.h"
#include "util/IdGenerator.h"

SampleService::SampleService(const std::string& filePath)
    : m_repo(std::make_unique<SampleRepository>(filePath))
{
}

bool SampleService::registerSample(const std::string& name, double avgProdTime, double yield) {
    if (!isValidInput(name, avgProdTime, yield)) {
        return false;
    }
    if (isDuplicateName(name)) {
        return false;
    }

    Sample s;
    s.sampleId    = IdGenerator::nextSampleId();
    s.name        = name;
    s.avgProdTime = avgProdTime;
    s.yield       = yield;
    s.stock       = 0;

    m_repo->add(s);
    m_repo->save();
    return true;
}

std::vector<Sample> SampleService::getAllSamples() const {
    return m_repo->findAll();
}

std::vector<Sample> SampleService::searchByName(const std::string& keyword) const {
    return m_repo->findByName(keyword);
}

std::optional<Sample> SampleService::getSampleById(const std::string& id) const {
    return m_repo->findById(id);
}

bool SampleService::isValidInput(const std::string& name, double avgProdTime, double yield) const {
    if (name.empty()) return false;
    if (avgProdTime <= 0.0) return false;
    if (yield <= 0.0 || yield > 1.0) return false;
    return true;
}

bool SampleService::isDuplicateName(const std::string& name) const {
    auto all = m_repo->findAll();
    for (const auto& s : all) {
        if (s.name == name) return true;
    }
    return false;
}
