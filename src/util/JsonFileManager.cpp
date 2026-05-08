#include "util/JsonFileManager.h"
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

nlohmann::json JsonFileManager::load(const std::string& path) {
    if (!fs::exists(path)) {
        return nlohmann::json::array();
    }

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return nlohmann::json::array();
    }

    try {
        nlohmann::json data;
        ifs >> data;
        return data;
    } catch (const nlohmann::json::exception&) {
        return nlohmann::json::array();
    }
}

void JsonFileManager::save(const std::string& path, const nlohmann::json& data) {
    fs::path filePath(path);
    if (filePath.has_parent_path()) {
        fs::create_directories(filePath.parent_path());
    }

    std::ofstream ofs(path);
    ofs << data.dump(2);
}
