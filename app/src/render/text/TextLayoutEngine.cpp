#include "TextLayoutEngine.h"
#include <algorithm>
#include <sstream>

namespace catchim::render {

TextBlockMeasurement TextLayoutEngine::measureTextBlock(
    const std::vector<double>& lineWidths,
    double lineHeightPx
) {
    if (lineWidths.empty()) {
        return TextBlockMeasurement{0.0, 0.0, 0.0, 0};
    }

    double maxWidth = 0.0;
    for (double w : lineWidths) {
        maxWidth = std::max(maxWidth, w);
    }

    size_t lineCount = lineWidths.size();
    double totalHeight = static_cast<double>(lineCount) * lineHeightPx;
    double visualCenterOffset = (static_cast<double>(lineCount - 1) * lineHeightPx) / 2.0;

    return TextBlockMeasurement{
        visualCenterOffset,
        totalHeight,
        maxWidth,
        lineCount
    };
}

TextRect TextLayoutEngine::getTextRect(
    TextAlignment alignment,
    const TextBlockMeasurement& block
) {
    double left = 0.0;
    switch (alignment) {
        case TextAlignment::Left:
            left = 0.0;
            break;
        case TextAlignment::Center:
            left = -block.maxWidth / 2.0;
            break;
        case TextAlignment::Right:
            left = -block.maxWidth;
            break;
    }

    double top = -block.height / 2.0;
    return TextRect{
        left,
        top,
        block.maxWidth,
        block.height
    };
}

TextRect TextLayoutEngine::computeTextBackgroundRect(
    const TextRect& textRect,
    double paddingX,
    double paddingY
) {
    return TextRect{
        textRect.left - paddingX,
        textRect.top - paddingY,
        textRect.width + 2.0 * paddingX,
        textRect.height + 2.0 * paddingY
    };
}

std::vector<std::string> TextLayoutEngine::wrapTextToLines(
    const std::string& text,
    double maxLineWidth,
    double averageCharWidth
) {
    if (text.empty()) {
        return {};
    }

    if (maxLineWidth <= 0.0 || averageCharWidth <= 0.0) {
        return {text};
    }

    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string rawLine;

    while (std::getline(ss, rawLine)) {
        if (rawLine.empty()) {
            lines.push_back("");
            continue;
        }

        std::stringstream wordStream(rawLine);
        std::string word;
        std::string currentLine;

        while (wordStream >> word) {
            double testWidth = static_cast<double>(currentLine.empty() ? word.length() : currentLine.length() + 1 + word.length()) * averageCharWidth;
            if (testWidth <= maxLineWidth || currentLine.empty()) {
                if (!currentLine.empty()) {
                    currentLine += " ";
                }
                currentLine += word;
            } else {
                lines.push_back(std::move(currentLine));
                currentLine = word;
            }
        }

        if (!currentLine.empty()) {
            lines.push_back(std::move(currentLine));
        }
    }

    return lines;
}

} // namespace catchim::render
