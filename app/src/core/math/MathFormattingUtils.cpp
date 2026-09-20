#include "core/math/MathFormattingUtils.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>

namespace catchim::core {

double MathFormattingUtils::clamp(double value, double min, double max) noexcept {
    return std::max(min, std::min(max, value));
}

int64_t MathFormattingUtils::clampRound(double value, double min, double max) noexcept {
    return static_cast<int64_t>(std::round(clamp(value, min, max)));
}

int MathFormattingUtils::getFractionDigitsForStep(double step) noexcept {
    if (step <= 0.0) return 0;
    std::ostringstream oss;
    oss << std::setprecision(10) << step;
    std::string str = oss.str();

    auto expPos = str.find("e-");
    if (expPos != std::string::npos) {
        return std::stoi(str.substr(expPos + 2));
    }

    auto dotPos = str.find('.');
    if (dotPos == std::string::npos) {
        return 0;
    }
    return static_cast<int>(str.length() - dotPos - 1);
}

double MathFormattingUtils::snapToStep(double value, double step) noexcept {
    if (step <= 0.0) return value;
    double snapped = std::round(value / step) * step;
    int digits = getFractionDigitsForStep(step);
    double factor = std::pow(10.0, digits);
    return std::round(snapped * factor) / factor;
}

bool MathFormattingUtils::isNearlyEqual(double left, double right, double epsilon) noexcept {
    return std::abs(left - right) <= epsilon;
}

std::string MathFormattingUtils::formatNumberForDisplay(
    double value,
    std::optional<int> fractionDigits,
    int minFractionDigits,
    int maxFractionDigits
) {
    int resolvedMax = std::max(0, fractionDigits.value_or(maxFractionDigits));
    int resolvedMin = std::min(std::max(0, fractionDigits.value_or(minFractionDigits)), resolvedMax);

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(resolvedMax) << value;
    std::string fixedValue = oss.str();

    if (resolvedMax == 0) {
        return (value == 0.0) ? "0" : fixedValue;
    }

    auto dotPos = fixedValue.find('.');
    std::string integerPart = (dotPos != std::string::npos) ? fixedValue.substr(0, dotPos) : fixedValue;
    std::string fractionPart = (dotPos != std::string::npos) ? fixedValue.substr(dotPos + 1) : "";

    // Normalize -0 to 0
    if (integerPart == "-0") {
        integerPart = "0";
    }

    while (static_cast<int>(fractionPart.length()) > resolvedMin && !fractionPart.empty() && fractionPart.back() == '0') {
        fractionPart.pop_back();
    }

    return fractionPart.empty() ? integerPart : (integerPart + "." + fractionPart);
}

static int gcdInternal(int a, int b) noexcept {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int r = a % b;
        a = b;
        b = r;
    }
    return a == 0 ? 1 : a;
}

std::string MathFormattingUtils::dimensionToAspectRatio(int width, int height) {
    if (width <= 0 || height <= 0) {
        return "1:1";
    }
    int divisor = gcdInternal(width, height);
    int aspectWidth = width / divisor;
    int aspectHeight = height / divisor;
    return std::to_string(aspectWidth) + ":" + std::to_string(aspectHeight);
}

} // namespace catchim::core
