#include "DateUtils.h"
#include <sstream>
#include <iomanip>
#include <array>

namespace catchim::core {

namespace {
constexpr std::array<const char*, 12> MONTH_NAMES = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
}

std::string DateUtils::formatDate(int year, int month, int day) {
    if (month < 1 || month > 12) {
        month = 1;
    }
    std::ostringstream oss;
    oss << MONTH_NAMES[static_cast<size_t>(month - 1)] << " " << day << ", " << year;
    return oss.str();
}

std::string DateUtils::formatDate(std::time_t time) {
    std::tm tmVal{};
#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tmVal, &time);
#else
    gmtime_r(&time, &tmVal);
#endif
    return formatDate(tmVal.tm_year + 1900, tmVal.tm_mon + 1, tmVal.tm_mday);
}

std::string DateUtils::formatDate(const std::chrono::system_clock::time_point& tp) {
    const std::time_t time = std::chrono::system_clock::to_time_t(tp);
    return formatDate(time);
}

} // namespace catchim::core
