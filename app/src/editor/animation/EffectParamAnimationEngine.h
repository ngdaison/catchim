#pragma once

#include "editor/timeline/Clip.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <nlohmann/json.hpp>

namespace catchim::editor {

/**
 * @brief Engine for constructing, parsing, and resolving keyframe animation paths on effect parameters.
 * Corresponds to web/src/animation/effect-param-channel.ts.
 */
class EffectParamAnimationEngine {
public:
    static constexpr std::string_view EFFECT_PARAM_PATH_PREFIX = "effects.";
    static constexpr std::string_view EFFECT_PARAM_PATH_SUFFIX = ".params.";

    /**
     * @brief Builds property path string for an effect parameter: "effects.<effectId>.params.<paramKey>"
     */
    static std::string buildEffectParamPath(
        const std::string& effectId,
        const std::string& paramKey
    );

    /**
     * @brief Checks if a property path corresponds to an effect parameter animation channel.
     */
    static bool isEffectParamPath(const std::string& propertyPath) noexcept;

    /**
     * @brief Parses an effect parameter property path into {effectId, paramKey}.
     */
    static std::optional<std::pair<std::string, std::string>> parseEffectParamPath(
        const std::string& propertyPath
    );

    /**
     * @brief Resolves all parameters of an effect at local clip time, evaluating animation
     * curves if keyframed or falling back to static parameter values.
     */
    static nlohmann::json resolveEffectParamsAtTime(
        const Clip& clip,
        const std::string& effectId,
        core::TimelineTime localTime
    );
};

} // namespace catchim::editor
