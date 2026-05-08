#include "repository/SampleRepository.h"
#include "util/JsonFileManager.h"

SampleRepository::SampleRepository(const std::string& filePath)
    : m_filePath(filePath), m_dirty(false)
{
    reload();
}

// 파일에서 강제로 다시 읽는다 (항상)
void SampleRepository::reload() const {
    m_samples.clear();
    auto json = JsonFileManager::load(m_filePath);
    for (const auto& item : json) m_samples.push_back(item.get<Sample>());
    m_dirty = false;
}

// dirty=false(저장 완료 상태)일 때만 파일에서 재로드한다.
// dirty=true(미저장 변경 있음)이면 현재 메모리 상태를 유지한다.
void SampleRepository::ensureFresh() const {
    if (!m_dirty) reload();
}

void SampleRepository::add(const Sample& sample) {
    ensureFresh(); // 다른 인스턴스가 저장한 최신 상태를 반영
    m_samples.push_back(sample);
    m_dirty = true;
}

std::vector<Sample> SampleRepository::findAll() const {
    ensureFresh();
    return m_samples;
}

std::optional<Sample> SampleRepository::findById(const std::string& id) const {
    ensureFresh();
    for (const auto& s : m_samples) {
        if (s.sampleId == id) return s;
    }
    return std::nullopt;
}

std::vector<Sample> SampleRepository::findByName(const std::string& keyword) const {
    ensureFresh();
    std::vector<Sample> result;
    for (const auto& s : m_samples) {
        if (s.name.find(keyword) != std::string::npos) result.push_back(s);
    }
    return result;
}

void SampleRepository::update(const Sample& sample) {
    ensureFresh();
    for (auto& s : m_samples) {
        if (s.sampleId == sample.sampleId) { s = sample; m_dirty = true; return; }
    }
}

void SampleRepository::save() const {
    JsonFileManager::save(m_filePath, nlohmann::json(m_samples));
    m_dirty = false; // 저장 완료 → 이후 read는 파일 재로드 허용
}
