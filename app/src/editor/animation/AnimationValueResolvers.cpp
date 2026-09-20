#include "AnimationValueResolvers.h"
#include "core/utils/ColorUtils.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

namespace {
constexpr double VALUE_EPSILON = 1e-6;

inline double clamp01(double val) noexcept {
    return std::clamp(val, 0.0, 1.0);
}
} // namespace

double AnimationValueResolvers::resolveOpacityAtTime(
    double baseOpacity,
    const AnimationChannel* channel,
    core::TimelineTime localTime
) noexcept {
    if (channel == nullptr || channel->empty()) {
        return baseOpacity;
    }
    const auto safeLocalTime = (localTime.ticks() < 0) ? core::TimelineTime(0) : localTime;
    return channel->getValueAt(safeLocalTime);
}

double AnimationValueResolvers::resolveNumberAtTime(
    double baseValue,
    const AnimationChannel* channel,
    core::TimelineTime localTime
) noexcept {
    if (channel == nullptr || channel->empty()) {
        return baseValue;
    }
    const auto safeLocalTime = (localTime.ticks() < 0) ? core::TimelineTime(0) : localTime;
    return channel->getValueAt(safeLocalTime);
}

std::string AnimationValueResolvers::resolveColorAtTime(
    const std::string& baseColor,
    const std::vector<AnimationChannel>& channels,
    core::TimelineTime localTime
) {
    if (channels.empty()) {
        return baseColor;
    }

    auto baseRgb = core::ColorUtils::hexToRgb(baseColor);
    if (!baseRgb.has_value()) {
        return baseColor;
    }

    const auto safeLocalTime = (localTime.ticks() < 0) ? core::TimelineTime(0) : localTime;

    core::RgbColor resolved = *baseRgb;
    for (const auto& ch : channels) {
        if (ch.empty()) continue;
        const auto& prop = ch.propertyName();
        const double val = ch.getValueAt(safeLocalTime);
        if (prop == "color.r" || prop == "r") {
            resolved.r = static_cast<uint8_t>(std::clamp(val, 0.0, 255.0));
        } else if (prop == "color.g" || prop == "g") {
            resolved.g = static_cast<uint8_t>(std::clamp(val, 0.0, 255.0));
        } else if (prop == "color.b" || prop == "b") {
            resolved.b = static_cast<uint8_t>(std::clamp(val, 0.0, 255.0));
        } else if (prop == "color.a" || prop == "a") {
            resolved.a = std::clamp(val, 0.0, 1.0);
        }
    }

    return core::ColorUtils::rgbToHex(resolved, resolved.a < 0.999);
}

std::optional<NormalizedCubicBezier> AnimationValueResolvers::getNormalizedCubicBezierForScalarSegment(
    const Keyframe& leftKey,
    const Keyframe& rightKey,
    std::optional<double> referenceSpanValue
) noexcept {
    const double spanTime = static_cast<double>((rightKey.time - leftKey.time).ticks());
    const double spanValue = rightKey.value - leftKey.value;

    double effectiveSpanValue = 0.0;
    if (std::abs(spanValue) > VALUE_EPSILON) {
        effectiveSpanValue = spanValue;
    } else if (referenceSpanValue.has_value() && std::abs(*referenceSpanValue) > VALUE_EPSILON) {
        effectiveSpanValue = *referenceSpanValue;
    } else {
        return std::nullopt;
    }

    if (spanTime <= 0.0) {
        return std::nullopt;
    }

    // Default right handle dt = 1/3 of spanTime, dv = 0
    const double rightHandleDt = (leftKey.rightHandle.x != 0.0) ? leftKey.rightHandle.x : (spanTime / 3.0);
    const double rightHandleDv = leftKey.rightHandle.y;

    // Default left handle dt = -1/3 of spanTime, dv = 0
    const double leftHandleDt = (rightKey.leftHandle.x != 0.0) ? rightKey.leftHandle.x : (-spanTime / 3.0);
    const double leftHandleDv = rightKey.leftHandle.y;

    return NormalizedCubicBezier{
        .x1 = clamp01(rightHandleDt / spanTime),
        .y1 = rightHandleDv / effectiveSpanValue,
        .x2 = clamp01(1.0 + (leftHandleDt / spanTime)),
        .y2 = 1.0 + (leftHandleDv / effectiveSpanValue)
    };
}

std::optional<CurveHandles> AnimationValueResolvers::getCurveHandlesForNormalizedCubicBezier(
    const Keyframe& leftKey,
    const Keyframe& rightKey,
    const NormalizedCubicBezier& bezier,
    std::optional<double> referenceSpanValue
) noexcept {
    const double spanTime = static_cast<double>((rightKey.time - leftKey.time).ticks());
    const double spanValue = rightKey.value - leftKey.value;

    double effectiveSpanValue = 0.0;
    if (std::abs(spanValue) > VALUE_EPSILON) {
        effectiveSpanValue = spanValue;
    } else if (referenceSpanValue.has_value() && std::abs(*referenceSpanValue) > VALUE_EPSILON) {
        effectiveSpanValue = *referenceSpanValue;
    } else {
        return std::nullopt;
    }

    if (spanTime <= 0.0) {
        return std::nullopt;
    }

    const double x1 = clamp01(bezier.x1);
    const double x2 = clamp01(bezier.x2);

    return CurveHandles{
        .rightHandle = KeyframeHandle{
            .x = spanTime * x1,
            .y = effectiveSpanValue * bezier.y1
        },
        .leftHandle = KeyframeHandle{
            .x = spanTime * (x2 - 1.0),
            .y = effectiveSpanValue * (bezier.y2 - 1.0)
        }
    };
}

} // namespace catchim::editor
