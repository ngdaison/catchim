#pragma once

#include <string>
#include <string_view>
#include <algorithm>
#include <cmath>

namespace catchim::render {

struct TextLayoutMetrics {
    double scaledFontSize{40.0};
    double lineHeightPx{48.0};
    double fontSizeRatio{2.66667};
    std::string fontString;

    bool operator==(const TextLayoutMetrics& other) const = default;
};

class TextRenderingPrimitives {
public:
    static constexpr double kFontSizeScaleReference = 1080.0;
    static constexpr double kCornerRadiusMin = 0.0;
    static constexpr double kCornerRadiusMax = 100.0;

    // Escapes and wraps font family name in quotes (e.g. "Inter" -> "\"Inter\"")
    static std::string quoteFontFamily(std::string_view fontFamily);

    // Formats a Canvas/CSS-compliant font specification string
    static std::string buildTextFontString(
        std::string_view fontFamily,
        std::string_view fontWeight,
        std::string_view fontStyle,
        double scaledFontSize
    );

    // Resolves scalable layout metrics adapted to the target canvas height (relative to 1080p reference)
    static TextLayoutMetrics resolveTextLayoutMetrics(
        double fontSize,
        double canvasHeight,
        double lineHeightRatio = 1.2,
        std::string_view fontFamily = "Inter",
        std::string_view fontWeight = "normal",
        std::string_view fontStyle = "normal"
    );

    // Calculates background corner radius in pixels from percentage [0, 100]% (max 50% of shortest edge)
    static double calculateCornerRadiusPx(
        double width,
        double height,
        double cornerRadiusPct
    ) noexcept;
};

} // namespace catchim::render
