#pragma once

#include "render/scene/SceneNodes.h"
#include "render/effects/BlurEffect.h"
#include "editor/animation/EffectParamAnimationEngine.h"
#include "core/time/TimelineTime.h"
#include <memory>
#include <vector>
#include <optional>
#include <string>

namespace catchim::render {

struct ResolvedVisualState {
    core::TimelineTime localTime{0};
    Transform2D transform;
    double opacity{1.0};
    std::vector<std::vector<EffectPass>> effectPassGroups;
    core::TimelineTime sourceTime{0};
};

struct ResolvedTextState {
    core::TimelineTime localTime{0};
    Transform2D transform;
    double opacity{1.0};
    std::string textColor{"#ffffff"};
    std::string backgroundColor{""};
    std::vector<std::vector<EffectPass>> effectPassGroups;
};

struct ResolvedEffectLayerState {
    std::vector<std::vector<EffectPass>> effectPassGroups;
};

struct ResolvedBlurBackgroundState {
    std::vector<EffectPass> passes;
};

/**
 * @brief Resolves dynamic scene node states at a specific timeline playhead time.
 * Corresponds to web/src/services/renderer/resolve.ts.
 */
class SceneTreeResolver {
public:
    static std::optional<ResolvedVisualState> resolveVisualNode(
        const VisualNode& node,
        core::TimelineTime time,
        double canvasWidth = 1920.0,
        double canvasHeight = 1080.0
    );

    static std::optional<ResolvedTextState> resolveTextNode(
        const TextNode& node,
        core::TimelineTime time,
        double canvasWidth = 1920.0,
        double canvasHeight = 1080.0
    );

    static std::optional<ResolvedBlurBackgroundState> resolveBlurBackgroundNode(
        const BlurBackgroundNode& node,
        core::TimelineTime time,
        double canvasWidth = 1920.0,
        double canvasHeight = 1080.0
    );

    static std::optional<ResolvedEffectLayerState> resolveEffectLayerNode(
        const EffectLayerNode& node,
        core::TimelineTime time,
        double canvasWidth = 1920.0,
        double canvasHeight = 1080.0
    );

    static std::vector<std::vector<EffectPass>> resolveEffectPassGroups(
        const nlohmann::json& effects,
        const std::vector<editor::AnimationChannel>& animations,
        core::TimelineTime localTime,
        double effectWidth,
        double effectHeight
    );
};

} // namespace catchim::render
