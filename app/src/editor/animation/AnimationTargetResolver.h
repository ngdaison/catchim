#pragma once

#include "editor/timeline/Clip.h"
#include "editor/animation/Keyframe.h"
#include <string>
#include <optional>
#include <functional>

namespace catchim::editor {

struct NumericRange {
    double min{0.0};
    double max{1.0};
    double step{0.01};
};

struct AnimationPathDescriptor {
    std::string channelLayout{"scalar"}; // "scalar" | "2d" | "color"
    KeyframeInterpolation defaultInterpolation{KeyframeInterpolation::Linear};
    std::optional<NumericRange> numericRange{std::nullopt};
    std::function<double(double)> coerceValue;
    std::function<double(const Clip&)> getBaseValue;
    std::function<void(Clip&, double)> setBaseValue;
};

/**
 * @brief Resolves animation target descriptors for clip parameters, graphic parameters, and effect parameters.
 * Corresponds to web/src/timeline/animation-targets.ts.
 */
class AnimationTargetResolver {
public:
    static std::optional<AnimationPathDescriptor> resolveAnimationTarget(
        const Clip& clip,
        const std::string& path
    );

private:
    static std::optional<AnimationPathDescriptor> buildElementParamDescriptor(
        const Clip& clip,
        const std::string& paramKey
    );

    static std::optional<AnimationPathDescriptor> buildGraphicParamDescriptor(
        const Clip& clip,
        const std::string& paramKey
    );

    static std::optional<AnimationPathDescriptor> buildEffectParamDescriptor(
        const Clip& clip,
        const std::string& effectId,
        const std::string& paramKey
    );
};

} // namespace catchim::editor
