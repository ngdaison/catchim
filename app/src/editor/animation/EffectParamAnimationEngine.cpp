#include "EffectParamAnimationEngine.h"
#include <algorithm>

namespace catchim::editor {

std::string EffectParamAnimationEngine::buildEffectParamPath(
    const std::string& effectId,
    const std::string& paramKey
) {
    return std::string(EFFECT_PARAM_PATH_PREFIX) + effectId + std::string(EFFECT_PARAM_PATH_SUFFIX) + paramKey;
}

bool EffectParamAnimationEngine::isEffectParamPath(const std::string& propertyPath) noexcept {
    return propertyPath.starts_with(EFFECT_PARAM_PATH_PREFIX) &&
           propertyPath.find(EFFECT_PARAM_PATH_SUFFIX) != std::string::npos;
}

std::optional<std::pair<std::string, std::string>> EffectParamAnimationEngine::parseEffectParamPath(
    const std::string& propertyPath
) {
    if (!isEffectParamPath(propertyPath)) {
        return std::nullopt;
    }

    std::string_view sv = propertyPath;
    sv.remove_prefix(EFFECT_PARAM_PATH_PREFIX.length());
    auto sepPos = sv.find(EFFECT_PARAM_PATH_SUFFIX);
    if (sepPos == std::string_view::npos || sepPos == 0) {
        return std::nullopt;
    }

    std::string effectId = std::string(sv.substr(0, sepPos));
    std::string paramKey = std::string(sv.substr(sepPos + EFFECT_PARAM_PATH_SUFFIX.length()));
    if (effectId.empty() || paramKey.empty()) {
        return std::nullopt;
    }

    return std::make_pair(std::move(effectId), std::move(paramKey));
}

nlohmann::json EffectParamAnimationEngine::resolveEffectParamsAtTime(
    const Clip& clip,
    const std::string& effectId,
    core::TimelineTime localTime
) {
    const auto& effects = clip.effects();
    auto it = std::find_if(effects.begin(), effects.end(), [&](const EffectInstance& eff) {
        return eff.id == effectId;
    });
    if (it == effects.end()) {
        return nlohmann::json::object();
    }

    nlohmann::json resolved = it->params;
    if (!resolved.is_object()) {
        resolved = nlohmann::json::object();
    }

    for (auto& [paramKey, value] : resolved.items()) {
        std::string path = buildEffectParamPath(effectId, paramKey);
        const auto* channel = clip.findAnimationChannel(path);
        if (channel && !channel->empty()) {
            double animVal = channel->getValueAt(localTime);
            value = animVal;
        }
    }

    return resolved;
}

} // namespace catchim::editor
