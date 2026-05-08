#include "repository/SampleRepository.h"
#include "util/JsonFileManager.h"
#include <algorithm>

SampleRepository::SampleRepository(const std::string& filePath)
    : m_filePath(filePath)
{
    auto json = JsonFileManager::load(m_filePath);
    for (const auto& item : json) {
        m_samples.push_back(item.get<Sample>());
    }
}

void SampleRepository::add(const Sample& sample) {
    m_samples.push_back(sample);
}

std::vector<Sample> SampleRepository::findAll() const {
    return m_samples;
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    for (const auto& s : m_samples) {
        if (s.sampleId == id) {
            return s;
        }
    }
    return std::nullopt;
}

std::vector<Sample> SampleRepository::findByName(const std::string& keyword) const {
    std::vector<Sample> result;
    for (const auto& s : m_samples) {
        if (s.name.find(keyword) != std::string::npos) {
            result.push_back(s);
        }
    }
    return result;
}

void SampleRepository::save() const {
    nlohmann::json json = nlohmann::json::array();
    for (const auto& s : m_samples) {
        json.push_back(s);
    }
    JsonFileManager::save(m_filePath, json);
}
