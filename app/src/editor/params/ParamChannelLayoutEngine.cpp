#include "editor/params/ParamChannelLayoutEngine.h"
#include "core/utils/ColorUtils.h"
#include "core/math/MathFormattingUtils.h"
#include <cstdio>
#include <limits>

namespace catchim::editor {

double ParamChannelLayoutEngine::srgbToLinear(double value) noexcept {
    if (value <= 0.04045) {
        return value / 12.92;
    }
    return std::pow((value + 0.055) / 1.055, 2.4);
}

double ParamChannelLayoutEngine::linearToSrgb(double value) noexcept {
    const double clamped = std::clamp(value, 0.0, 1.0);
    if (clamped <= 0.0031308) {
        return clamped * 12.92;
    }
    return 1.055 * std::pow(clamped, 1.0 / 2.4) - 0.055;
}

std::optional<LinearRgba> ParamChannelLayoutEngine::parseColorToLinearRgba(const std::string& color) {
    const auto rgb = core::ColorUtils::hexToRgb(color);
    if (!rgb.has_value()) {
        return std::nullopt;
    }

    const double rNorm = static_cast<double>(rgb->r) / 255.0;
    const double gNorm = static_cast<double>(rgb->g) / 255.0;
    const double bNorm = static_cast<double>(rgb->b) / 255.0;

    return LinearRgba{
        .r = srgbToLinear(rNorm),
        .g = srgbToLinear(gNorm),
        .b = srgbToLinear(bNorm),
        .a = std::clamp(rgb->a, 0.0, 1.0)
    };
}

std::string ParamChannelLayoutEngine::formatLinearRgba(const LinearRgba& color) {
    const double sR = linearToSrgb(color.r);
    const double sG = linearToSrgb(color.g);
    const double sB = linearToSrgb(color.b);
    const double sA = std::clamp(color.a, 0.0, 1.0);

    const auto uR = static_cast<uint8_t>(std::clamp(std::round(sR * 255.0), 0.0, 255.0));
    const auto uG = static_cast<uint8_t>(std::clamp(std::round(sG * 255.0), 0.0, 255.0));
    const auto uB = static_cast<uint8_t>(std::clamp(std::round(sB * 255.0), 0.0, 255.0));

    if (sA < 1.0 - 1e-4) {
        const auto uA = static_cast<uint8_t>(std::clamp(std::round(sA * 255.0), 0.0, 255.0));
        char buf[10];
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", uR, uG, uB, uA);
        return std::string(buf);
    } else {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", uR, uG, uB);
        return std::string(buf);
    }
}

double ParamChannelLayoutEngine::coerceParamValueNumber(
    double value,
    double min,
    std::optional<double> max,
    double step
) noexcept {
    const double stepped = core::MathFormattingUtils::snapToStep(value, step);
    const double maxVal = max.has_value() ? *max : std::numeric_limits<double>::infinity();
    return std::min(maxVal, std::max(min, stepped));
}

std::optional<std::string> ParamChannelLayoutEngine::coerceParamValueSelect(
    const std::string& value,
    const std::vector<std::string>& validOptions
) {
    for (const auto& opt : validOptions) {
        if (opt == value) {
            return value;
        }
    }
    return std::nullopt;
}

} // namespace catchim::editor
