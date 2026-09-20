#pragma once

#include <string>
#include <chrono>
#include <ctime>

namespace catchim::core {

class DateUtils {
public:
    static std::string formatDate(const std::chrono::system_clock::time_point& tp);
    static std::string formatDate(std::time_t time);
    static std::string formatDate(int year, int month, int day);
};

} // namespace catchim::core
