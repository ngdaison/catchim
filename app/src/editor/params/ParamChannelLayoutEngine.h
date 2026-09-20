#pragma once

#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <cmath>

namespace catchim::editor {

struct LinearRgba {
    double r{0.0}; // [0, 1] linear
    double g{0.0}; // [0, 1] linear
    double b{0.0}; // [0, 1] linear
    double a{1.0}; // [0, 1]

    bool operator==(const LinearRgba& other) const = default;
};

enum class ParamValueKind {
    Number,
    Color,
    Discrete
};

enum class ParamDefaultInterpolation {
    Linear,
    Hold
};

/**
 * @brief Engine for param channel layouts, sRGB <-> Linear conversions, and value coercion.
 * Corresponds to web/src/params/index.ts.
 */
class ParamChannelLayoutEngine {
public:
    static double srgbToLinear(double value) noexcept;
    static double linearToSrgb(double value) noexcept;

    static std::optional<LinearRgba> parseColorToLinearRgba(const std::string& color);
    static std::string formatLinearRgba(const LinearRgba& color);

    static double coerceParamValueNumber(
        double value,
        double min,
        std::optional<double> max,
        double step
    ) noexcept;

    static std::optional<std::string> coerceParamValueSelect(
        const std::string& value,
        const std::vector<std::string>& validOptions
    );
};

} // namespace catchim::editor
