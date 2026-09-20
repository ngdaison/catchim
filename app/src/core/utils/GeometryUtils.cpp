#include "GeometryUtils.h"
#include <numeric>
#include <cmath>

namespace catchim::core {

int64_t GeometryUtils::gcd(int64_t a, int64_t b) noexcept {
    a = std::abs(a);
    b = std::abs(b);
    return std::gcd(a, b);
}

std::string GeometryUtils::dimensionToAspectRatio(int width, int height) {
    return dimensionToAspectRatio(static_cast<int64_t>(width), static_cast<int64_t>(height));
}

std::string GeometryUtils::dimensionToAspectRatio(int64_t width, int64_t height) {
    if (width <= 0 || height <= 0) {
        return "0:0";
    }
    const int64_t divisor = gcd(width, height);
    const int64_t aspectWidth = width / (divisor == 0 ? 1 : divisor);
    const int64_t aspectHeight = height / (divisor == 0 ? 1 : divisor);
    return std::to_string(aspectWidth) + ":" + std::to_string(aspectHeight);
}

std::string GeometryUtils::dimensionToAspectRatio(double width, double height) {
    if (width <= 0.0 || height <= 0.0 || !std::isfinite(width) || !std::isfinite(height)) {
        return "0:0";
    }
    return dimensionToAspectRatio(
        static_cast<int64_t>(std::round(width)),
        static_cast<int64_t>(std::round(height))
    );
}

} // namespace catchim::core
