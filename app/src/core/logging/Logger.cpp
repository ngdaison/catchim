#include "Logger.h"

namespace catchim::core {

Logger& Logger::instance() {
    static Logger s_instance;
    return s_instance;
}

void Logger::log(LogLevel level, std::string_view message, const char* file, int line) {
    if (level < minLevel_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    const char* levelStr = "INFO";
    switch (level) {
        case LogLevel::Trace:    levelStr = "TRACE"; break;
        case LogLevel::Debug:    levelStr = "DEBUG"; break;
        case LogLevel::Info:     levelStr = "INFO"; break;
        case LogLevel::Warning:  levelStr = "WARN"; break;
        case LogLevel::Error:    levelStr = "ERROR"; break;
        case LogLevel::Critical: levelStr = "CRITICAL"; break;
    }

    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &now_time_t);
#else
    localtime_r(&now_time_t, &tm_buf);
#endif

    char time_str[32];
    std::strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_buf);

    if (file) {
        // Strip path to only filename
        std::string_view file_sv(file);
        auto last_slash = file_sv.find_last_of("/\\");
        if (last_slash != std::string_view::npos) {
            file_sv = file_sv.substr(last_slash + 1);
        }
        std::cout << "[" << time_str << "] [" << levelStr << "] [" << file_sv << ":" << line << "] "
                  << message << std::endl;
    } else {
        std::cout << "[" << time_str << "] [" << levelStr << "] " << message << std::endl;
    }
}

} // namespace catchim::core
