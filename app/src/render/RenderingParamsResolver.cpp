#include "render/RenderingParamsResolver.h"
#include <algorithm>

namespace catchim::render {

static const std::vector<std::string> s_allBlendModes = {
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

const std::vector<std::string>& RenderingParamsResolver::allBlendModes() noexcept {
    return s_allBlendModes;
}

std::string RenderingParamsResolver::blendModeToString(BlendMode mode) {
    switch (mode) {
        case BlendMode::Normal: return "normal";
        case BlendMode::Darken: return "darken";
        case BlendMode::Multiply: return "multiply";
        case BlendMode::ColorBurn: return "color-burn";
        case BlendMode::Lighten: return "lighten";
        case BlendMode::Screen: return "screen";
        case BlendMode::PlusLighter: return "plus-lighter";
        case BlendMode::ColorDodge: return "color-dodge";
        case BlendMode::Overlay: return "overlay";
        case BlendMode::SoftLight: return "soft-light";
        case BlendMode::HardLight: return "hard-light";
        case BlendMode::Difference: return "difference";
        case BlendMode::Exclusion: return "exclusion";
        case BlendMode::Hue: return "hue";
        case BlendMode::Saturation: return "saturation";
        case BlendMode::Color: return "color";
        case BlendMode::Luminosity: return "luminosity";
    }
    return "normal";
}

std::optional<BlendMode> RenderingParamsResolver::blendModeFromString(const std::string& str) {
    if (str == "normal") return BlendMode::Normal;
    if (str == "darken") return BlendMode::Darken;
    if (str == "multiply") return BlendMode::Multiply;
    if (str == "color-burn") return BlendMode::ColorBurn;
    if (str == "lighten") return BlendMode::Lighten;
    if (str == "screen") return BlendMode::Screen;
    if (str == "plus-lighter") return BlendMode::PlusLighter;
    if (str == "color-dodge") return BlendMode::ColorDodge;
    if (str == "overlay") return BlendMode::Overlay;
    if (str == "soft-light") return BlendMode::SoftLight;
    if (str == "hard-light") return BlendMode::HardLight;
    if (str == "difference") return BlendMode::Difference;
    if (str == "exclusion") return BlendMode::Exclusion;
    if (str == "hue") return BlendMode::Hue;
    if (str == "saturation") return BlendMode::Saturation;
    if (str == "color") return BlendMode::Color;
    if (str == "luminosity") return BlendMode::Luminosity;
    return std::nullopt;
}

bool RenderingParamsResolver::isBlendMode(const std::string& value) noexcept {
    return std::find(s_allBlendModes.begin(), s_allBlendModes.end(), value) != s_allBlendModes.end();
}

static double readNumber(const std::unordered_map<std::string, double>& params, const std::string& key, double fallback) noexcept {
    auto it = params.find(key);
    if (it != params.end()) {
        return it->second;
    }
    return fallback;
}

RenderTransform RenderingParamsResolver::buildTransformFromParams(
    const std::unordered_map<std::string, double>& params
) noexcept {
    RenderTransform t;
    t.scaleX = readNumber(params, "transform.scaleX", 1.0);
    t.scaleY = readNumber(params, "transform.scaleY", 1.0);
    t.position.x = readNumber(params, "transform.positionX", 0.0);
    t.position.y = readNumber(params, "transform.positionY", 0.0);
    t.rotate = readNumber(params, "transform.rotate", 0.0);
    return t;
}

double RenderingParamsResolver::readOpacityFromParams(
    const std::unordered_map<std::string, double>& params,
    double fallback
) noexcept {
    return readNumber(params, "opacity", fallback);
}

std::string RenderingParamsResolver::readBlendModeFromParams(
    const std::unordered_map<std::string, std::string>& params,
    const std::string& fallback
) {
    auto it = params.find("blendMode");
    if (it != params.end() && isBlendMode(it->second)) {
        return it->second;
    }
    return fallback;
}

} // namespace catchim::render
