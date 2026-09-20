#pragma once

#include <string>

namespace catchim::core {

class StringUtils {
public:
    static std::string capitalizeFirstLetter(const std::string& str);
    static std::string uppercase(const std::string& str);
    static std::string lowercase(const std::string& str);

    static bool isAppleDevice() noexcept;
    static std::string getPlatformSpecialKey();
    static std::string getPlatformAlternateKey();
};

} // namespace catchim::core
