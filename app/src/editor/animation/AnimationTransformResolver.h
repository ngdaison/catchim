#pragma once

#include "editor/timeline/Clip.h"
#include "core/time/TimelineTime.h"

namespace catchim::editor {

struct AnimatedTransform {
    double positionX{0.0};
    double positionY{0.0};
    double scaleX{1.0};
    double scaleY{1.0};
    double rotate{0.0};

    bool operator==(const AnimatedTransform& other) const = default;
};

/**
 * @brief Computes dynamic transform, opacity, and volume at a specific clip-local time.
 * Evaluates animation channels with fallback to clip base parameters.
 * Corresponds to web/src/rendering/animation-values.ts.
 */
class AnimationTransformResolver {
public:
    static AnimatedTransform resolveTransformAtTime(
        const Clip& clip,
        core::TimelineTime localTime
    );

    static double resolveOpacityAtTime(
        const Clip& clip,
        core::TimelineTime localTime
    );

    static double resolveVolumeAtTime(
        const Clip& clip,
        core::TimelineTime localTime
    );

private:
    static double resolveChannelValue(
        const Clip& clip,
        const std::string& primaryKey,
        const std::string& aliasKey,
        core::TimelineTime localTime,
        double fallbackValue
    );
};

} // namespace catchim::editor
