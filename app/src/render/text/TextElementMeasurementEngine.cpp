#include "TextElementMeasurementEngine.h"
#include "editor/timeline/Clip.h"
#include <cmath>
#include <algorithm>

namespace catchim::render {

TextBackground TextElementMeasurementEngine::buildTextBackgroundFromParams(
    const std::unordered_map<std::string, std::string>& params
) {
    TextBackground bg;

    auto itEnabled = params.find("background.enabled");
    if (itEnabled != params.end()) {
        bg.enabled = (itEnabled->second == "true" || itEnabled->second == "1");
    }

    auto itColor = params.find("background.color");
    if (itColor != params.end()) {
        bg.color = itColor->second;
    }

    auto itRadius = params.find("background.cornerRadius");
    if (itRadius != params.end()) {
        try {
            bg.cornerRadius = std::clamp(std::stod(itRadius->second), CORNER_RADIUS_MIN, CORNER_RADIUS_MAX);
        } catch (...) {}
    }

    auto itPadX = params.find("background.paddingX");
    if (itPadX != params.end()) {
        try {
            bg.paddingX = std::stod(itPadX->second);
        } catch (...) {}
    }

    auto itPadY = params.find("background.paddingY");
    if (itPadY != params.end()) {
        try {
            bg.paddingY = std::stod(itPadY->second);
        } catch (...) {}
    }

    auto itOffX = params.find("background.offsetX");
    if (itOffX != params.end()) {
        try {
            bg.offsetX = std::stod(itOffX->second);
        } catch (...) {}
    }

    auto itOffY = params.find("background.offsetY");
    if (itOffY != params.end()) {
        try {
            bg.offsetY = std::stod(itOffY->second);
        } catch (...) {}
    }

    return bg;
}

ResolvedTextBackground TextElementMeasurementEngine::resolveBackgroundAtTime(
    const TextBackground& base,
    const editor::Clip* clip,
    core::TimelineTime localTime
) {
    ResolvedTextBackground resolved{
        .enabled = base.enabled,
        .color = base.color,
        .cornerRadius = base.cornerRadius,
        .paddingX = base.paddingX,
        .paddingY = base.paddingY,
        .offsetX = base.offsetX,
        .offsetY = base.offsetY
    };

    if (!clip) {
        return resolved;
    }

    if (const auto* ch = clip->findAnimationChannel("background.paddingX")) {
        resolved.paddingX = ch->getValueAt(localTime);
    }
    if (const auto* ch = clip->findAnimationChannel("background.paddingY")) {
        resolved.paddingY = ch->getValueAt(localTime);
    }
    if (const auto* ch = clip->findAnimationChannel("background.offsetX")) {
        resolved.offsetX = ch->getValueAt(localTime);
    }
    if (const auto* ch = clip->findAnimationChannel("background.offsetY")) {
        resolved.offsetY = ch->getValueAt(localTime);
    }
    if (const auto* ch = clip->findAnimationChannel("background.cornerRadius")) {
        resolved.cornerRadius = std::clamp(ch->getValueAt(localTime), CORNER_RADIUS_MIN, CORNER_RADIUS_MAX);
    }

    return resolved;
}

TextVisualRect TextElementMeasurementEngine::calculateVisualRect(
    const TextVisualRect& textBlockRect,
    const ResolvedTextBackground& bg
) {
    if (!bg.enabled) {
        return textBlockRect;
    }

    return TextVisualRect{
        .left = textBlockRect.left - bg.paddingX + bg.offsetX,
        .top = textBlockRect.top - bg.paddingY + bg.offsetY,
        .width = textBlockRect.width + (2.0 * bg.paddingX),
        .height = textBlockRect.height + (2.0 * bg.paddingY)
    };
}

MeasuredTextElement TextElementMeasurementEngine::measureElement(
    const std::unordered_map<std::string, std::string>& params,
    const editor::Clip* clip,
    core::TimelineTime localTime,
    double textWidth,
    double textHeight
) {
    TextBackground baseBg = buildTextBackgroundFromParams(params);
    ResolvedTextBackground resolvedBg = resolveBackgroundAtTime(baseBg, clip, localTime);

    TextVisualRect baseRect{
        .left = -textWidth * 0.5,
        .top = -textHeight * 0.5,
        .width = textWidth,
        .height = textHeight
    };

    TextVisualRect visual = calculateVisualRect(baseRect, resolvedBg);

    return MeasuredTextElement{
        .textWidth = textWidth,
        .textHeight = textHeight,
        .resolvedBackground = resolvedBg,
        .visualRect = visual
    };
}

} // namespace catchim::render
