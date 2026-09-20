#include "RenderParamResolvers.h"
#include <array>
#include <algorithm>

namespace catchim::render {

namespace {
constexpr std::array<const char*, 17> VALID_BLEND_MODES = {
    "normal",
    "darken",
    "multiply",
    "color-burn",
    "lighten",
    "screen",
    "plus-lighter",
    "color-dodge",
    "overlay",
    "soft-light",
    "hard-light",
    "difference",
    "exclusion",
    "hue",
    "saturation",
    "color",
    "luminosity"
};
}

bool RenderParamResolvers::isBlendMode(const std::string& mode) noexcept {
    for (const auto* valid : VALID_BLEND_MODES) {
        if (mode == valid) {
            return true;
        }
    }
    return false;
}

std::string RenderParamResolvers::readBlendModeFromParams(const nlohmann::json& params) {
    if (params.is_object() && params.contains("blendMode") && params["blendMode"].is_string()) {
        const std::string mode = params["blendMode"].get<std::string>();
        if (isBlendMode(mode)) {
            return mode;
        }
    }
    return "normal";
}

double RenderParamResolvers::readOpacityFromParams(const nlohmann::json& params) noexcept {
    if (params.is_object() && params.contains("opacity") && params["opacity"].is_number()) {
        return params["opacity"].get<double>();
    }
    return 1.0;
}

Transform RenderParamResolvers::buildTransformFromParams(const nlohmann::json& params) {
    Transform transform;
    if (!params.is_object()) {
        return transform;
    }

    if (params.contains("transform.scaleX") && params["transform.scaleX"].is_number()) {
        transform.scaleX = params["transform.scaleX"].get<double>();
    }
    if (params.contains("transform.scaleY") && params["transform.scaleY"].is_number()) {
        transform.scaleY = params["transform.scaleY"].get<double>();
    }
    if (params.contains("transform.positionX") && params["transform.positionX"].is_number()) {
        transform.positionX = params["transform.positionX"].get<double>();
    }
    if (params.contains("transform.positionY") && params["transform.positionY"].is_number()) {
        transform.positionY = params["transform.positionY"].get<double>();
    }
    if (params.contains("transform.rotate") && params["transform.rotate"].is_number()) {
        transform.rotate = params["transform.rotate"].get<double>();
    }

    transform.opacity = readOpacityFromParams(params);
    transform.blendMode = readBlendModeFromParams(params);
    return transform;
}

Transform RenderParamResolvers::resolveTransformAtTime(
    const Transform& baseTransform,
    const std::vector<editor::AnimationChannel>& channels,
    core::TimelineTime localTime
) {
    Transform resolved = baseTransform;
    const auto safeLocalTime = (localTime.ticks() < 0) ? core::TimelineTime(0) : localTime;

    for (const auto& channel : channels) {
        if (channel.empty()) continue;
        const auto& prop = channel.propertyName();
        const double val = channel.getValueAt(safeLocalTime);

        if (prop == "transform.positionX") {
            resolved.positionX = val;
        } else if (prop == "transform.positionY") {
            resolved.positionY = val;
        } else if (prop == "transform.scaleX") {
            resolved.scaleX = val;
        } else if (prop == "transform.scaleY") {
            resolved.scaleY = val;
        } else if (prop == "transform.rotate") {
            resolved.rotate = val;
        } else if (prop == "opacity") {
            resolved.opacity = val;
        }
    }

    return resolved;
}

} // namespace catchim::render
