#include "render/scene/SceneTreeResolver.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

std::vector<std::vector<EffectPass>> SceneTreeResolver::resolveEffectPassGroups(
    const nlohmann::json& effects,
    const std::vector<editor::AnimationChannel>& animations,
    core::TimelineTime localTime,
    double effectWidth,
    double effectHeight
) {
    if (!effects.is_array()) {
        return {};
    }

    std::vector<std::vector<EffectPass>> groups;

    for (const auto& eff : effects) {
        bool enabled = eff.value("enabled", true);
        if (!enabled) {
            continue;
        }

        std::string effId = eff.value("id", "");
        std::string effType = eff.value("type", "blur");
        nlohmann::json params = eff.value("params", nlohmann::json::object());

        nlohmann::json resolvedParams = params;
        if (!resolvedParams.is_object()) {
            resolvedParams = nlohmann::json::object();
        }
        for (auto& [paramKey, val] : resolvedParams.items()) {
            std::string path = editor::EffectParamAnimationEngine::buildEffectParamPath(effId, paramKey);
            for (const auto& chan : animations) {
                if (chan.propertyName() == path && !chan.empty()) {
                    val = chan.getValueAt(localTime);
                    break;
                }
            }
        }

        if (effType == "blur") {
            float intensity = 15.0f;
            if (resolvedParams.contains("intensity")) {
                if (resolvedParams["intensity"].is_number()) {
                    intensity = resolvedParams["intensity"].get<float>();
                }
            }
            float sigmaX = BlurEffect::intensityToSigma(intensity, static_cast<float>(effectWidth), 1920.0f);
            float sigmaY = BlurEffect::intensityToSigma(intensity, static_cast<float>(effectHeight), 1080.0f);
            auto passes = BlurEffect::buildGaussianBlurPasses(sigmaX, sigmaY);
            if (!passes.empty()) {
                groups.push_back(std::move(passes));
            }
        }
    }

    return groups;
}

std::optional<ResolvedVisualState> SceneTreeResolver::resolveVisualNode(
    const VisualNode& node,
    core::TimelineTime time,
    double canvasWidth,
    double canvasHeight
) {
    core::TimelineTime clipTime = time - node.timeOffset();
    if (clipTime < core::TimelineTime(0) || clipTime >= node.duration()) {
        return std::nullopt;
    }

    core::TimelineTime localTime = clipTime;

    Transform2D transform = node.transform();
    for (const auto& chan : node.animations()) {
        if (chan.empty()) continue;
        const auto& prop = chan.propertyName();
        if (prop == "transform.positionX" || prop == "position_x" || prop == "positionX") {
            transform.positionX = chan.getValueAt(localTime);
        } else if (prop == "transform.positionY" || prop == "position_y" || prop == "positionY") {
            transform.positionY = chan.getValueAt(localTime);
        } else if (prop == "transform.scaleX" || prop == "scale_x" || prop == "scaleX") {
            transform.scaleX = chan.getValueAt(localTime);
        } else if (prop == "transform.scaleY" || prop == "scale_y" || prop == "scaleY") {
            transform.scaleY = chan.getValueAt(localTime);
        } else if (prop == "transform.rotate" || prop == "rotate" || prop == "rotation") {
            transform.rotate = chan.getValueAt(localTime);
        }
    }

    double opacity = node.opacity();
    for (const auto& chan : node.animations()) {
        if (chan.propertyName() == "opacity" && !chan.empty()) {
            opacity = chan.getValueAt(localTime);
            break;
        }
    }

    core::TimelineTime sourceTime = node.trimStart() + clipTime;
    if (node.type() == SceneNodeType::Video) {
        const auto& vn = static_cast<const VideoNode&>(node);
        if (vn.retimeRate() > 0.0) {
            sourceTime = node.trimStart() + core::TimelineTime(static_cast<int64_t>(clipTime.ticks() * vn.retimeRate()));
        }
    }

    double effectW = canvasWidth * std::abs(transform.scaleX);
    double effectH = canvasHeight * std::abs(transform.scaleY);
    auto effectPasses = resolveEffectPassGroups(
        node.effects(),
        node.animations(),
        localTime,
        effectW,
        effectH
    );

    return ResolvedVisualState{
        .localTime = localTime,
        .transform = transform,
        .opacity = std::clamp(opacity, 0.0, 1.0),
        .effectPassGroups = std::move(effectPasses),
        .sourceTime = sourceTime
    };
}

std::optional<ResolvedTextState> SceneTreeResolver::resolveTextNode(
    const TextNode& node,
    core::TimelineTime time,
    double canvasWidth,
    double canvasHeight
) {
    core::TimelineTime clipTime = time - node.timeOffset();
    if (clipTime < core::TimelineTime(0) || clipTime >= node.duration()) {
        return std::nullopt;
    }

    core::TimelineTime localTime = clipTime;

    Transform2D transform = node.transform();
    for (const auto& chan : node.animations()) {
        if (chan.empty()) continue;
        const auto& prop = chan.propertyName();
        if (prop == "transform.positionX" || prop == "position_x" || prop == "positionX") {
            transform.positionX = chan.getValueAt(localTime);
        } else if (prop == "transform.positionY" || prop == "position_y" || prop == "positionY") {
            transform.positionY = chan.getValueAt(localTime);
        } else if (prop == "transform.scaleX" || prop == "scale_x" || prop == "scaleX") {
            transform.scaleX = chan.getValueAt(localTime);
        } else if (prop == "transform.scaleY" || prop == "scale_y" || prop == "scaleY") {
            transform.scaleY = chan.getValueAt(localTime);
        } else if (prop == "transform.rotate" || prop == "rotate" || prop == "rotation") {
            transform.rotate = chan.getValueAt(localTime);
        }
    }

    double opacity = node.opacity();
    for (const auto& chan : node.animations()) {
        if (chan.propertyName() == "opacity" && !chan.empty()) {
            opacity = chan.getValueAt(localTime);
            break;
        }
    }

    auto effectPasses = resolveEffectPassGroups(
        node.effects(),
        node.animations(),
        localTime,
        canvasWidth,
        canvasHeight
    );

    return ResolvedTextState{
        .localTime = localTime,
        .transform = transform,
        .opacity = std::clamp(opacity, 0.0, 1.0),
        .textColor = node.textColor(),
        .backgroundColor = node.backgroundEnabled() ? node.backgroundColor() : "",
        .effectPassGroups = std::move(effectPasses)
    };
}

std::optional<ResolvedBlurBackgroundState> SceneTreeResolver::resolveBlurBackgroundNode(
    const BlurBackgroundNode& node,
    core::TimelineTime time,
    double canvasWidth,
    double canvasHeight
) {
    core::TimelineTime clipTime = time - node.timeOffset();
    if (clipTime < core::TimelineTime(0) || clipTime >= node.duration()) {
        return std::nullopt;
    }

    float sigmaX = BlurEffect::intensityToSigma(
        static_cast<float>(node.blurIntensity()),
        static_cast<float>(canvasWidth),
        1920.0f
    );
    float sigmaY = BlurEffect::intensityToSigma(
        static_cast<float>(node.blurIntensity()),
        static_cast<float>(canvasHeight),
        1080.0f
    );

    return ResolvedBlurBackgroundState{
        .passes = BlurEffect::buildGaussianBlurPasses(sigmaX, sigmaY)
    };
}

std::optional<ResolvedEffectLayerState> SceneTreeResolver::resolveEffectLayerNode(
    const EffectLayerNode& node,
    core::TimelineTime time,
    double canvasWidth,
    double canvasHeight
) {
    core::TimelineTime clipTime = time - node.timeOffset();
    if (clipTime < core::TimelineTime(0) || clipTime >= node.duration()) {
        return std::nullopt;
    }

    nlohmann::json effectsArr = nlohmann::json::array({
        {
            {"id", "scene_effect"},
            {"type", node.effectType()},
            {"params", node.effectParams()},
            {"enabled", true}
        }
    });

    auto passes = resolveEffectPassGroups(
        effectsArr,
        {},
        clipTime,
        canvasWidth,
        canvasHeight
    );

    if (passes.empty()) {
        return std::nullopt;
    }

    return ResolvedEffectLayerState{
        .effectPassGroups = std::move(passes)
    };
}

} // namespace catchim::render
