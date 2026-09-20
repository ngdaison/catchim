#pragma once

#include "Keyframe.h"
#include "AnimationChannel.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::editor {

struct NormalizedCubicBezier {
    double x1{0.0};
    double y1{0.0};
    double x2{1.0};
    double y2{1.0};

    constexpr bool operator==(const NormalizedCubicBezier& other) const noexcept = default;
};

struct CurveHandles {
    KeyframeHandle rightHandle;
    KeyframeHandle leftHandle;
};

class AnimationValueResolvers {
public:
    static double resolveOpacityAtTime(
        double baseOpacity,
        const AnimationChannel* channel,
        core::TimelineTime localTime
    ) noexcept;

    static double resolveNumberAtTime(
        double baseValue,
        const AnimationChannel* channel,
        core::TimelineTime localTime
    ) noexcept;

    static std::string resolveColorAtTime(
        const std::string& baseColor,
        const std::vector<AnimationChannel>& channels,
        core::TimelineTime localTime
    );

    static std::optional<NormalizedCubicBezier> getNormalizedCubicBezierForScalarSegment(
        const Keyframe& leftKey,
        const Keyframe& rightKey,
        std::optional<double> referenceSpanValue = std::nullopt
    ) noexcept;

    static std::optional<CurveHandles> getCurveHandlesForNormalizedCubicBezier(
        const Keyframe& leftKey,
        const Keyframe& rightKey,
        const NormalizedCubicBezier& bezier,
        std::optional<double> referenceSpanValue = std::nullopt
    ) noexcept;
};

} // namespace catchim::editor
