#pragma once

#include <string>
#include <string_view>
#include <sstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <format>

namespace catchim::core {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical
};

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level) { minLevel_ = level; }
    LogLevel level() const { return minLevel_; }

    void log(LogLevel level, std::string_view message, const char* file = nullptr, int line = 0);

    template <typename... Args>
    void logFormatted(LogLevel level, const char* file, int line, std::format_string<Args...> fmt, Args&&... args) {
        if (level < minLevel_) return;
        std::string msg = std::format(fmt, std::forward<Args>(args)...);
        log(level, msg, file, line);
    }

private:
    Logger() = default;
    ~Logger() = default;

    LogLevel minLevel_{LogLevel::Info};
    std::mutex mutex_;
};

} // namespace catchim::core

#define LOG_TRACE(fmt, ...) ::catchim::core::Logger::instance().logFormatted(::catchim::core::LogLevel::Trace, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) ::catchim::core::Logger::instance().logFormatted(::catchim::core::LogLevel::Debug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  ::catchim::core::Logger::instance().logFormatted(::catchim::core::LogLevel::Info,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  ::catchim::core::Logger::instance().logFormatted(::catchim::core::LogLevel::Warning, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) ::catchim::core::Logger::instance().logFormatted(::catchim::core::LogLevel::Error, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) ::catchim::core::Logger::instance().logFormatted(::catchim::core::LogLevel::Critical, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
