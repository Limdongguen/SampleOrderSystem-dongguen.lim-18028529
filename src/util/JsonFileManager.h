#pragma once
#include <string>
#include <nlohmann/json.hpp>

class JsonFileManager {
public:
    static nlohmann::json load(const std::string& path);
    static void           save(const std::string& path, const nlohmann::json& data);
};
