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
    void update(const Sample& sample);
    void save() const;

private:
    std::string                 m_filePath;
    mutable std::vector<Sample> m_samples;
    mutable bool                m_dirty = false; // true: 미저장 변경 있음 → read 시 파일 로드 안 함

    void reload() const;
    void ensureFresh() const; // dirty=false 일 때만 파일에서 재로드
};
