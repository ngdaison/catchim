#pragma once

#include <string>
#include <filesystem>

namespace catchim::app {

struct AppConfig {
    int windowWidth{1440};
    int windowHeight{900};
    bool maximized{true};
    std::string theme{"dark"}; // "dark" | "light"
    std::string language{"vi"}; // "vi" | "en"
    std::string lastProjectPath{""};

    static AppConfig load();
    void save() const;

    static std::filesystem::path getConfigFilePath();
};

} // namespace catchim::app
