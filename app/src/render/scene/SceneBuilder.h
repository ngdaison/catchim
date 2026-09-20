#pragma once

#include "render/scene/SceneNodes.h"
#include "editor/timeline/Track.h"
#include "editor/timeline/Clip.h"
#include "media/MediaAsset.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace catchim::render {

struct SceneBackgroundConfig {
    std::string type{"color"}; // "color" | "blur"
    std::string color{"#000000"};
    double blurIntensity{15.0};
};

struct SceneBuilderParams {
    double canvasWidth{1920.0};
    double canvasHeight{1080.0};
    double coordinateScale{1.0};
    std::vector<editor::Track> tracks;
    std::vector<std::shared_ptr<media::MediaAsset>> mediaAssets;
    core::TimelineTime duration{0};
    SceneBackgroundConfig background;
    bool isPreview{false};
};

/**
 * @brief Constructs a renderable SceneNode tree from timeline tracks and media assets.
 * Corresponds to web/src/services/renderer/scene-builder.ts.
 */
class SceneBuilder {
public:
    static std::shared_ptr<RootNode> buildScene(const SceneBuilderParams& params);

    static std::vector<std::shared_ptr<SceneNode>> buildTrackNodes(
        const std::vector<editor::Track>& tracks,
        const std::unordered_map<std::string, std::shared_ptr<media::MediaAsset>>& mediaMap,
        double canvasWidth,
        double canvasHeight,
        double coordinateScale = 1.0,
        bool isPreview = false
    );

    static std::vector<std::shared_ptr<SceneNode>> buildBlurBackgroundNodes(
        const editor::Track* mainTrack,
        const std::unordered_map<std::string, std::shared_ptr<media::MediaAsset>>& mediaMap,
        double blurIntensity
    );

    static Transform2D buildScaledTransformFromClip(
        const editor::Clip& clip,
        double coordinateScale = 1.0
    );
};

} // namespace catchim::render
