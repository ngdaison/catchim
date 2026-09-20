#pragma once

#include <string>
#include <vector>

namespace catchim::render {

enum class TextAlignment {
    Left,
    Center,
    Right
};

struct TextRect {
    double left{0.0};
    double top{0.0};
    double width{0.0};
    double height{0.0};

    bool operator==(const TextRect& other) const noexcept {
        return left == other.left && top == other.top && width == other.width && height == other.height;
    }
};

struct TextBlockMeasurement {
    double visualCenterOffset{0.0};
    double height{0.0};
    double maxWidth{0.0};
    size_t lineCount{0};
};

struct TextBackgroundParams {
    double paddingX{0.0};
    double paddingY{0.0};
    double cornerRadius{0.0};
    std::string color{"#000000"};
    double opacity{1.0};
};

/**
 * @brief Multi-line text measurement, alignment, background bounds, and wrapping engine.
 * Corresponds to web/src/text/layout.ts and primitives.ts.
 */
class TextLayoutEngine {
public:
    static TextBlockMeasurement measureTextBlock(
        const std::vector<double>& lineWidths,
        double lineHeightPx
    );

    static TextRect getTextRect(
        TextAlignment alignment,
        const TextBlockMeasurement& block
    );

    static TextRect computeTextBackgroundRect(
        const TextRect& textRect,
        double paddingX,
        double paddingY
    );

    static std::vector<std::string> wrapTextToLines(
        const std::string& text,
        double maxLineWidth,
        double averageCharWidth = 10.0
    );
};

} // namespace catchim::render
