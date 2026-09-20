#include "render/scene/SceneBuilder.h"
#include <algorithm>

namespace catchim::render {

Transform2D SceneBuilder::buildScaledTransformFromClip(
    const editor::Clip& clip,
    double coordinateScale
) {
    Transform2D t;
    t.positionX = clip.getParam<double>("positionX", 0.0) * coordinateScale;
    t.positionY = clip.getParam<double>("positionY", 0.0) * coordinateScale;
    t.scaleX = clip.getParam<double>("scaleX", 1.0);
    t.scaleY = clip.getParam<double>("scaleY", 1.0);
    t.rotate = clip.getParam<double>("rotate", 0.0);
    return t;
}

std::vector<std::shared_ptr<SceneNode>> SceneBuilder::buildBlurBackgroundNodes(
    const editor::Track* mainTrack,
    const std::unordered_map<std::string, std::shared_ptr<media::MediaAsset>>& mediaMap,
    double blurIntensity
) {
    if (!mainTrack) {
        return {};
    }

    std::vector<editor::Clip> sortedClips;
    for (const auto& clip : mainTrack->clips()) {
        if (!clip.isHidden() && (clip.type() == editor::ClipType::Video || clip.type() == editor::ClipType::Image)) {
            sortedClips.push_back(clip);
        }
    }

    std::sort(sortedClips.begin(), sortedClips.end(), [](const editor::Clip& a, const editor::Clip& b) {
        if (a.startTime() != b.startTime()) {
            return a.startTime() < b.startTime();
        }
        return a.id().str() < b.id().str();
    });

    std::vector<std::shared_ptr<SceneNode>> nodes;
    for (const auto& clip : sortedClips) {
        auto it = mediaMap.find(clip.mediaId().str());
        if (it == mediaMap.end() || !it->second) {
            continue;
        }
        const auto& asset = it->second;
        std::string mediaTypeStr = (clip.type() == editor::ClipType::Video) ? "video" : "image";

        nodes.push_back(std::make_shared<BlurBackgroundNode>(
            asset->id().str(),
            asset->filePath().string(),
            mediaTypeStr,
            clip.duration(),
            clip.startTime(),
            clip.trimStart(),
            clip.trimEnd(),
            blurIntensity
        ));
    }

    return nodes;
}

std::vector<std::shared_ptr<SceneNode>> SceneBuilder::buildTrackNodes(
    const std::vector<editor::Track>& tracks,
    const std::unordered_map<std::string, std::shared_ptr<media::MediaAsset>>& mediaMap,
    double /*canvasWidth*/,
    double /*canvasHeight*/,
    double coordinateScale,
    bool isPreview
) {
    std::vector<std::shared_ptr<SceneNode>> nodes;

    for (const auto& track : tracks) {
        if (track.isHidden()) {
            continue;
        }

        std::vector<editor::Clip> sortedClips;
        for (const auto& clip : track.clips()) {
            if (!clip.isHidden()) {
                sortedClips.push_back(clip);
            }
        }

        std::sort(sortedClips.begin(), sortedClips.end(), [](const editor::Clip& a, const editor::Clip& b) {
            if (a.startTime() != b.startTime()) {
                return a.startTime() < b.startTime();
            }
            return a.id().str() < b.id().str();
        });

        for (const auto& clip : sortedClips) {
            if (clip.type() == editor::ClipType::Effect) {
                std::string effectType = clip.getParam<std::string>("effectType", "blur");
                nodes.push_back(std::make_shared<EffectLayerNode>(
                    effectType,
                    clip.params(),
                    clip.startTime(),
                    clip.duration()
                ));
                continue;
            }

            Transform2D transform = buildScaledTransformFromClip(clip, coordinateScale);
            double opacity = clip.getParam<double>("opacity", 1.0);
            std::string blendMode = clip.getParam<std::string>("blendMode", "normal");

            std::vector<editor::AnimationChannel> anims;
            for (const auto& [prop, channel] : clip.animationChannels()) {
                anims.push_back(channel);
            }

            nlohmann::json effectsJson = nlohmann::json::array();
            for (const auto& eff : clip.effects()) {
                effectsJson.push_back({
                    {"id", eff.id},
                    {"type", eff.type},
                    {"params", eff.params},
                    {"enabled", eff.enabled}
                });
            }

            nlohmann::json masksJson = nlohmann::json::array();
            for (const auto& m : clip.masks()) {
                masksJson.push_back({
                    {"id", m.id},
                    {"type", m.type},
                    {"params", m.params}
                });
            }

            if (clip.type() == editor::ClipType::Video) {
                auto it = mediaMap.find(clip.mediaId().str());
                std::string url = (it != mediaMap.end() && it->second) ? it->second->filePath().string() : "";
                double retimeRate = clip.getParam<double>("retimeRate", 1.0);

                nodes.push_back(std::make_shared<VideoNode>(
                    clip.id().str(),
                    clip.mediaId().str(),
                    url,
                    clip.duration(),
                    clip.startTime(),
                    clip.trimStart(),
                    clip.trimEnd(),
                    transform,
                    opacity,
                    blendMode,
                    anims,
                    effectsJson,
                    masksJson,
                    retimeRate
                ));
            } else if (clip.type() == editor::ClipType::Image) {
                auto it = mediaMap.find(clip.mediaId().str());
                std::string url = (it != mediaMap.end() && it->second) ? it->second->filePath().string() : "";
                double maxSourceSize = isPreview ? 2048.0 : 4096.0;

                nodes.push_back(std::make_shared<ImageNode>(
                    clip.id().str(),
                    clip.mediaId().str(),
                    url,
                    clip.duration(),
                    clip.startTime(),
                    clip.trimStart(),
                    clip.trimEnd(),
                    transform,
                    opacity,
                    blendMode,
                    anims,
                    effectsJson,
                    masksJson,
                    maxSourceSize
                ));
            } else if (clip.type() == editor::ClipType::Text) {
                std::string content = clip.getParam<std::string>("content", "");
                std::string fontFamily = clip.getParam<std::string>("fontFamily", "Arial");
                double fontSize = clip.getParam<double>("fontSize", 15.0);
                std::string fontWeight = clip.getParam<std::string>("fontWeight", "normal");
                std::string fontStyle = clip.getParam<std::string>("fontStyle", "normal");
                std::string textAlign = clip.getParam<std::string>("textAlign", "center");
                std::string textDecoration = clip.getParam<std::string>("textDecoration", "none");
                double letterSpacing = clip.getParam<double>("letterSpacing", 0.0);
                double lineHeight = clip.getParam<double>("lineHeight", 1.2);
                std::string textColor = clip.getParam<std::string>("color", "#ffffff");
                std::string backgroundColor = clip.getParam<std::string>("backgroundColor", "");
                bool backgroundEnabled = clip.getParam<bool>("backgroundEnabled", false);

                nodes.push_back(std::make_shared<TextNode>(
                    clip.id().str(),
                    clip.duration(),
                    clip.startTime(),
                    content,
                    fontFamily,
                    fontSize,
                    fontWeight,
                    fontStyle,
                    textAlign,
                    textDecoration,
                    letterSpacing,
                    lineHeight,
                    textColor,
                    backgroundColor,
                    backgroundEnabled,
                    transform,
                    opacity,
                    blendMode,
                    anims,
                    effectsJson
                ));
            } else if (clip.type() == editor::ClipType::Sticker) {
                std::string stickerId = clip.getParam<std::string>("stickerId", "");
                double intrinsicWidth = clip.getParam<double>("intrinsicWidth", 256.0);
                double intrinsicHeight = clip.getParam<double>("intrinsicHeight", 256.0);

                nodes.push_back(std::make_shared<StickerNode>(
                    clip.id().str(),
                    stickerId,
                    intrinsicWidth,
                    intrinsicHeight,
                    clip.duration(),
                    clip.startTime(),
                    clip.trimStart(),
                    clip.trimEnd(),
                    transform,
                    opacity,
                    blendMode,
                    anims,
                    effectsJson
                ));
            } else if (clip.type() == editor::ClipType::Graphic) {
                std::string definitionId = clip.getParam<std::string>("definitionId", "");
                nodes.push_back(std::make_shared<GraphicNode>(
                    clip.id().str(),
                    definitionId,
                    clip.params(),
                    clip.duration(),
                    clip.startTime(),
                    clip.trimStart(),
                    clip.trimEnd(),
                    transform,
                    opacity,
                    blendMode,
                    anims,
                    effectsJson,
                    masksJson
                ));
            }
        }
    }

    return nodes;
}

std::shared_ptr<RootNode> SceneBuilder::buildScene(const SceneBuilderParams& params) {
    auto root = std::make_shared<RootNode>(params.duration);

    std::unordered_map<std::string, std::shared_ptr<media::MediaAsset>> mediaMap;
    for (const auto& asset : params.mediaAssets) {
        if (asset) {
            mediaMap[asset->id().str()] = asset;
        }
    }

    const editor::Track* mainTrack = nullptr;
    std::vector<editor::Track> visibleTracks;

    for (const auto& track : params.tracks) {
        if (track.isHidden()) {
            continue;
        }
        if (!mainTrack && (track.type() == editor::TrackType::Video)) {
            mainTrack = &track;
        }
        visibleTracks.push_back(track);
    }

    // Background setup
    if (params.background.type == "blur") {
        auto blurNodes = buildBlurBackgroundNodes(mainTrack, mediaMap, params.background.blurIntensity);
        for (const auto& node : blurNodes) {
            root->addChild(node);
        }
    } else if (params.background.type == "color" &&
               params.background.color != "transparent" &&
               !params.background.color.empty()) {
        root->addChild(std::make_shared<ColorNode>(params.background.color));
    }

    // Build track nodes
    auto trackNodes = buildTrackNodes(
        visibleTracks,
        mediaMap,
        params.canvasWidth,
        params.canvasHeight,
        params.coordinateScale,
        params.isPreview
    );

    for (const auto& node : trackNodes) {
        root->addChild(node);
    }

    return root;
}

} // namespace catchim::render
