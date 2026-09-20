#include "render/text/TextRenderingPrimitives.h"
#include <sstream>

namespace catchim::render {

std::string TextRenderingPrimitives::quoteFontFamily(std::string_view fontFamily) {
    std::string result;
    result.push_back('"');
    for (char c : fontFamily) {
        if (c == '"') {
            result.push_back('\\');
        }
        result.push_back(c);
    }
    result.push_back('"');
    return result;
}

std::string TextRenderingPrimitives::buildTextFontString(
    std::string_view fontFamily,
    std::string_view fontWeight,
    std::string_view fontStyle,
    double scaledFontSize
) {
    std::string cleanWeight = fontWeight.empty() ? "normal" : std::string(fontWeight);
    std::string cleanStyle = fontStyle.empty() ? "normal" : std::string(fontStyle);
    int sizePx = static_cast<int>(std::round(scaledFontSize));

    std::ostringstream oss;
    oss << cleanStyle << " " << cleanWeight << " " << sizePx << "px "
        << quoteFontFamily(fontFamily) << ", sans-serif";
    return oss.str();
}

TextLayoutMetrics TextRenderingPrimitives::resolveTextLayoutMetrics(
    double fontSize,
    double canvasHeight,
    double lineHeightRatio,
    std::string_view fontFamily,
    std::string_view fontWeight,
    std::string_view fontStyle
) {
    double ref = (canvasHeight > 0.0) ? canvasHeight : kFontSizeScaleReference;
    double scaledFontSize = fontSize * (ref / kFontSizeScaleReference);
    double lineHeightPx = scaledFontSize * lineHeightRatio;
    double fontSizeRatio = fontSize / 15.0;

    std::string fontString = buildTextFontString(fontFamily, fontWeight, fontStyle, scaledFontSize);

    return TextLayoutMetrics{
        scaledFontSize,
        lineHeightPx,
        fontSizeRatio,
        std::move(fontString)
    };
}

double TextRenderingPrimitives::calculateCornerRadiusPx(
    double width,
    double height,
    double cornerRadiusPct
) noexcept {
    if (width <= 0.0 || height <= 0.0) {
        return 0.0;
    }
    double p = std::clamp(cornerRadiusPct, kCornerRadiusMin, kCornerRadiusMax) / 100.0;
    return (std::min(width, height) / 2.0) * p;
}

} // namespace catchim::render
