#pragma once

#include <string>
#include <cstdint>

namespace catchim::core {

class GeometryUtils {
public:
    static int64_t gcd(int64_t a, int64_t b) noexcept;
    static std::string dimensionToAspectRatio(int width, int height);
    static std::string dimensionToAspectRatio(int64_t width, int64_t height);
    static std::string dimensionToAspectRatio(double width, double height);
};

} // namespace catchim::core
