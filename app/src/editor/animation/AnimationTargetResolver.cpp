#include "editor/animation/AnimationTargetResolver.h"
#include "editor/animation/EffectParamAnimationEngine.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {

double clampWithRange(double val, const std::optional<NumericRange>& range) {
    if (range.has_value()) {
        return std::clamp(val, range->min, range->max);
    }
    return val;
}

} // namespace

std::optional<AnimationPathDescriptor> AnimationTargetResolver::buildElementParamDescriptor(
    const Clip& /*clip*/,
    const std::string& paramKey
) {
    if (paramKey == "position_x" || paramKey == "transform.positionX") {
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = std::nullopt,
            .coerceValue = [](double v) { return v; },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("position_x", 0.0); },
            .setBaseValue = [](Clip& c, double v) { c.setParam("position_x", v); }
        };
    }
    if (paramKey == "position_y" || paramKey == "transform.positionY") {
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = std::nullopt,
            .coerceValue = [](double v) { return v; },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("position_y", 0.0); },
            .setBaseValue = [](Clip& c, double v) { c.setParam("position_y", v); }
        };
    }
    if (paramKey == "scale_x" || paramKey == "transform.scaleX") {
        NumericRange range{.min = 0.01, .max = 100.0, .step = 0.01};
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = range,
            .coerceValue = [range](double v) { return clampWithRange(v, range); },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("scale_x", 1.0); },
            .setBaseValue = [range](Clip& c, double v) { c.setParam("scale_x", clampWithRange(v, range)); }
        };
    }
    if (paramKey == "scale_y" || paramKey == "transform.scaleY") {
        NumericRange range{.min = 0.01, .max = 100.0, .step = 0.01};
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = range,
            .coerceValue = [range](double v) { return clampWithRange(v, range); },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("scale_y", 1.0); },
            .setBaseValue = [range](Clip& c, double v) { c.setParam("scale_y", clampWithRange(v, range)); }
        };
    }
    if (paramKey == "rotate" || paramKey == "transform.rotate") {
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = std::nullopt,
            .coerceValue = [](double v) { return v; },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("rotate", 0.0); },
            .setBaseValue = [](Clip& c, double v) { c.setParam("rotate", v); }
        };
    }
    if (paramKey == "opacity") {
        NumericRange range{.min = 0.0, .max = 1.0, .step = 0.01};
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = range,
            .coerceValue = [range](double v) { return clampWithRange(v, range); },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("opacity", 1.0); },
            .setBaseValue = [range](Clip& c, double v) { c.setParam("opacity", clampWithRange(v, range)); }
        };
    }
    if (paramKey == "volume") {
        NumericRange range{.min = 0.0, .max = 2.0, .step = 0.01};
        return AnimationPathDescriptor{
            .channelLayout = "scalar",
            .defaultInterpolation = KeyframeInterpolation::Linear,
            .numericRange = range,
            .coerceValue = [range](double v) { return clampWithRange(v, range); },
            .getBaseValue = [](const Clip& c) { return c.getParam<double>("volume", 1.0); },
            .setBaseValue = [range](Clip& c, double v) { c.setParam("volume", clampWithRange(v, range)); }
        };
    }

    return std::nullopt;
}

std::optional<AnimationPathDescriptor> AnimationTargetResolver::buildGraphicParamDescriptor(
    const Clip& clip,
    const std::string& paramKey
) {
    if (clip.type() != ClipType::Graphic) {
        return std::nullopt;
    }

    std::string cleanKey = paramKey;
    if (cleanKey.starts_with("graphics.")) {
        cleanKey = cleanKey.substr(9);
    }

    if (!clip.params().contains(cleanKey)) {
        return std::nullopt;
    }

    return AnimationPathDescriptor{
        .channelLayout = "scalar",
        .defaultInterpolation = KeyframeInterpolation::Linear,
        .numericRange = std::nullopt,
        .coerceValue = [](double v) { return v; },
        .getBaseValue = [cleanKey](const Clip& c) {
            return c.getParam<double>(cleanKey, 0.0);
        },
        .setBaseValue = [cleanKey](Clip& c, double v) {
            c.setParam(cleanKey, v);
        }
    };
}

std::optional<AnimationPathDescriptor> AnimationTargetResolver::buildEffectParamDescriptor(
    const Clip& clip,
    const std::string& effectId,
    const std::string& paramKey
) {
    const auto& effects = clip.effects();
    auto it = std::find_if(effects.begin(), effects.end(), [&](const EffectInstance& eff) {
        return eff.id == effectId;
    });

    if (it == effects.end()) {
        return std::nullopt;
    }

    return AnimationPathDescriptor{
        .channelLayout = "scalar",
        .defaultInterpolation = KeyframeInterpolation::Linear,
        .numericRange = std::nullopt,
        .coerceValue = [](double v) { return v; },
        .getBaseValue = [effectId, paramKey](const Clip& c) {
            for (const auto& eff : c.effects()) {
                if (eff.id == effectId && eff.params.contains(paramKey)) {
                    if (eff.params[paramKey].is_number()) {
                        return eff.params[paramKey].get<double>();
                    }
                }
            }
            return 0.0;
        },
        .setBaseValue = [effectId, paramKey](Clip& c, double v) {
            auto effs = c.effects();
            for (auto& eff : effs) {
                if (eff.id == effectId) {
                    eff.params[paramKey] = v;
                    break;
                }
            }
            c.setEffects(std::move(effs));
        }
    };
}

std::optional<AnimationPathDescriptor> AnimationTargetResolver::resolveAnimationTarget(
    const Clip& clip,
    const std::string& path
) {
    // 1. Element param
    auto elementTarget = buildElementParamDescriptor(clip, path);
    if (elementTarget.has_value()) {
        return elementTarget;
    }

    // 2. Graphic param
    auto graphicTarget = buildGraphicParamDescriptor(clip, path);
    if (graphicTarget.has_value()) {
        return graphicTarget;
    }

    // 3. Effect param
    auto effectParsed = EffectParamAnimationEngine::parseEffectParamPath(path);
    if (effectParsed.has_value()) {
        return buildEffectParamDescriptor(clip, effectParsed->first, effectParsed->second);
    }

    return std::nullopt;
}

} // namespace catchim::editor
