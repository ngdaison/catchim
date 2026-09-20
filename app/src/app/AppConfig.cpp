#include "AppConfig.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace catchim::app {

std::filesystem::path AppConfig::getConfigFilePath() {
    std::filesystem::path baseDir;
#if defined(_WIN32)
    const char* appData = std::getenv("LOCALAPPDATA");
    if (appData) {
        baseDir = std::filesystem::path(appData) / "Catchim";
    } else {
        baseDir = std::filesystem::current_path();
    }
#else
    const char* home = std::getenv("HOME");
    if (home) {
        baseDir = std::filesystem::path(home) / ".catchim";
    } else {
        baseDir = std::filesystem::current_path();
    }
#endif
    std::error_code ec;
    std::filesystem::create_directories(baseDir, ec);
    return baseDir / "config.json";
}

AppConfig AppConfig::load() {
    AppConfig config;
    auto path = getConfigFilePath();
    std::error_code ec;
    if (!std::filesystem::exists(path, ec)) return config;

    try {
        std::ifstream f(path);
        if (f.is_open()) {
            nlohmann::json j;
            f >> j;
            config.windowWidth = j.value("windowWidth", 1440);
            config.windowHeight = j.value("windowHeight", 900);
            config.maximized = j.value("maximized", true);
            config.theme = j.value("theme", "dark");
            config.language = j.value("language", "vi");
            config.lastProjectPath = j.value("lastProjectPath", "");
        }
    } catch (...) {}
    return config;
}

void AppConfig::save() const {
    auto path = getConfigFilePath();
    try {
        nlohmann::json j;
        j["windowWidth"] = windowWidth;
        j["windowHeight"] = windowHeight;
        j["maximized"] = maximized;
        j["theme"] = theme;
        j["language"] = language;
        j["lastProjectPath"] = lastProjectPath;

        std::ofstream f(path);
        if (f.is_open()) {
            f << j.dump(2);
        }
    } catch (...) {}
}

} // namespace catchim::app
