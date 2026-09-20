#include "editor/subtitles/SubtitleElementBuilder.h"
#include <sstream>
#include <algorithm>

namespace catchim::editor {

double SubtitleElementBuilder::resolveTargetWidth(
    double canvasWidth,
    const SubtitlePlacement& placement
) {
    double leftRatio = placement.marginLeftRatio.value_or(0.0);
    double rightRatio = placement.marginRightRatio.value_or(0.0);
    bool hasExplicitMargins = (leftRatio > 0.0 || rightRatio > 0.0);

    if (!hasExplicitMargins) {
        return canvasWidth * SUBTITLE_MAX_WIDTH_RATIO;
    }

    double availableWidth = canvasWidth * (1.0 - leftRatio - rightRatio);
    return std::max(0.0, availableWidth);
}

double SubtitleElementBuilder::resolvePositionX(
    double canvasWidth,
    SubtitleTextAlign textAlign,
    const SubtitlePlacement& placement,
    double approxTextWidth
) {
    double leftMargin = canvasWidth * placement.marginLeftRatio.value_or(0.0);
    double rightMargin = canvasWidth * placement.marginRightRatio.value_or(0.0);
    double canvasCenterX = canvasWidth / 2.0;

    if (textAlign == SubtitleTextAlign::Left) {
        return leftMargin + approxTextWidth / 2.0 - canvasCenterX;
    }

    if (textAlign == SubtitleTextAlign::Right) {
        return (canvasWidth - rightMargin) - approxTextWidth / 2.0 - canvasCenterX;
    }

    // Center alignment
    double availableWidth = canvasWidth - leftMargin - rightMargin;
    double targetCenterX = leftMargin + availableWidth / 2.0;
    return targetCenterX - canvasCenterX;
}

double SubtitleElementBuilder::resolvePositionY(
    double canvasHeight,
    const SubtitlePlacement& placement,
    double approxTextHeight
) {
    double margin = canvasHeight * placement.marginVerticalRatio.value_or(SUBTITLE_BOTTOM_MARGIN_RATIO);
    double canvasCenterY = canvasHeight / 2.0;

    if (placement.verticalAlign == SubtitleVerticalAlign::Top) {
        return margin + approxTextHeight / 2.0 - canvasCenterY;
    }

    if (placement.verticalAlign == SubtitleVerticalAlign::Middle) {
        return 0.0;
    }

    // Bottom
    return (canvasHeight - margin) - approxTextHeight / 2.0 - canvasCenterY;
}

std::string SubtitleElementBuilder::wrapSubtitleText(
    const std::string& text,
    double maxWidth,
    double approxCharWidth
) {
    if (text.empty() || maxWidth <= 0.0 || approxCharWidth <= 0.0) {
        return text;
    }

    std::istringstream stream(text);
    std::string paragraph;
    std::string result;

    bool firstParagraph = true;
    while (std::getline(stream, paragraph)) {
        if (!firstParagraph) {
            result += '\n';
        }
        firstParagraph = false;

        std::istringstream wordStream(paragraph);
        std::string word;
        std::string currentLine;

        while (wordStream >> word) {
            std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
            double candidateWidth = static_cast<double>(candidate.size()) * approxCharWidth;

            if (candidateWidth <= maxWidth || currentLine.empty()) {
                currentLine = candidate;
            } else {
                if (!result.empty() && result.back() != '\n') {
                    result += '\n';
                }
                result += currentLine;
                currentLine = word;
            }
        }

        if (!currentLine.empty()) {
            if (!result.empty() && result.back() != '\n' && !firstParagraph) {
                result += '\n';
            }
            result += currentLine;
        }
    }

    return result;
}

Clip SubtitleElementBuilder::buildSubtitleTextElement(
    size_t index,
    const AssSubtitleCue& cue,
    double canvasWidth,
    double canvasHeight
) {
    double fontSize = (cue.style.fontSizeRatioOfPlayHeight > 0.0)
        ? (cue.style.fontSizeRatioOfPlayHeight * FONT_SIZE_SCALE_REFERENCE)
        : cue.style.fontSize;

    double approxCharWidth = fontSize * 0.5;
    double targetWidth = resolveTargetWidth(canvasWidth, cue.style.placement);
    std::string content = wrapSubtitleText(cue.text, targetWidth, approxCharWidth);

    // Estimate line count for vertical centering of the block
    size_t lineCount = 1;
    for (char c : content) {
        if (c == '\n') lineCount++;
    }
    double lineHeightPx = fontSize * 1.2;
    double approxTextHeight = static_cast<double>(lineCount) * lineHeightPx;
    double approxTextWidth = std::min(targetWidth, static_cast<double>(content.size()) * approxCharWidth);

    double posX = resolvePositionX(canvasWidth, cue.style.textAlign, cue.style.placement, approxTextWidth);
    double posY = resolvePositionY(canvasHeight, cue.style.placement, approxTextHeight);

    std::string alignStr = "center";
    if (cue.style.textAlign == SubtitleTextAlign::Left) alignStr = "left";
    else if (cue.style.textAlign == SubtitleTextAlign::Right) alignStr = "right";

    Clip clip(
        core::ClipId::generate(),
        ClipType::Text,
        "Caption " + std::to_string(index + 1),
        cue.startTime,
        cue.duration
    );

    nlohmann::json params;
    params["content"] = content;
    params["fontSize"] = fontSize;
    params["fontFamily"] = cue.style.fontFamily;
    params["color"] = cue.style.color;
    params["opacity"] = cue.style.opacity;
    params["textAlign"] = alignStr;
    params["fontWeight"] = cue.style.bold ? "bold" : "normal";
    params["fontStyle"] = cue.style.italic ? "italic" : "normal";
    params["textDecoration"] = cue.style.underline ? "underline" : "none";
    params["transform.positionX"] = posX;
    params["transform.positionY"] = posY;
    params["transform.scaleX"] = 1.0;
    params["transform.scaleY"] = 1.0;
    params["transform.rotate"] = 0.0;

    clip.setParams(params);
    return clip;
}

} // namespace catchim::editor
