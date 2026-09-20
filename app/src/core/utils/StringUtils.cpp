#include "StringUtils.h"
#include <cctype>
#include <algorithm>

namespace catchim::core {

std::string StringUtils::capitalizeFirstLetter(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    std::string result = str;
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    return result;
}

std::string StringUtils::uppercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return result;
}

std::string StringUtils::lowercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

bool StringUtils::isAppleDevice() noexcept {
#if defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

std::string StringUtils::getPlatformSpecialKey() {
    return isAppleDevice() ? "\xE2\x8C\x98" : "Ctrl"; // UTF-8 "⌘" is \xE2\x8C\x98
}

std::string StringUtils::getPlatformAlternateKey() {
    return isAppleDevice() ? "\xE2\x8C\xA5" : "Alt"; // UTF-8 "⌥" is \xE2\x8C\xA5
}

} // namespace catchim::core
