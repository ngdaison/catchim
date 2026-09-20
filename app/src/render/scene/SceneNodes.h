#pragma once

#include "core/time/TimelineTime.h"
#include "editor/animation/AnimationChannel.h"
#include "render/canvas/CanvasTransformPipeline.h"
#include "render/effects/BlurEffect.h"
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace catchim::render {

enum class SceneNodeType {
    Root,
    Color,
    BlurBackground,
    EffectLayer,
    Video,
    Image,
    Text,
    Sticker,
    Graphic
};

/**
 * @brief Base class for all nodes in the rendering scene graph.
 * Corresponds to web/src/services/renderer/nodes/base-node.ts.
 */
class SceneNode {
public:
    virtual ~SceneNode() = default;
    virtual SceneNodeType type() const = 0;

    const std::vector<std::shared_ptr<SceneNode>>& children() const noexcept { return children_; }
    void addChild(std::shared_ptr<SceneNode> child) { children_.push_back(std::move(child)); }
    void removeChild(const std::shared_ptr<SceneNode>& child) {
        std::erase(children_, child);
    }
    void clearChildren() noexcept { children_.clear(); }

protected:
    std::vector<std::shared_ptr<SceneNode>> children_;
};

class RootNode : public SceneNode {
public:
    explicit RootNode(core::TimelineTime duration = core::TimelineTime(0))
        : duration_(duration) {}

    SceneNodeType type() const override { return SceneNodeType::Root; }

    core::TimelineTime duration() const noexcept { return duration_; }
    void setDuration(core::TimelineTime dur) noexcept { duration_ = dur; }

private:
    core::TimelineTime duration_;
};

class ColorNode : public SceneNode {
public:
    explicit ColorNode(std::string color = "#000000")
        : color_(std::move(color)) {}

    SceneNodeType type() const override { return SceneNodeType::Color; }

    const std::string& color() const noexcept { return color_; }
    void setColor(std::string color) { color_ = std::move(color); }

private:
    std::string color_;
};

class BlurBackgroundNode : public SceneNode {
public:
    BlurBackgroundNode(
        std::string mediaId,
        std::string url,
        std::string mediaType,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        core::TimelineTime trimStart,
        core::TimelineTime trimEnd,
        double blurIntensity = 15.0
    ) : mediaId_(std::move(mediaId)),
        url_(std::move(url)),
        mediaType_(std::move(mediaType)),
        duration_(duration),
        timeOffset_(timeOffset),
        trimStart_(trimStart),
        trimEnd_(trimEnd),
        blurIntensity_(blurIntensity) {}

    SceneNodeType type() const override { return SceneNodeType::BlurBackground; }

    const std::string& mediaId() const noexcept { return mediaId_; }
    const std::string& url() const noexcept { return url_; }
    const std::string& mediaType() const noexcept { return mediaType_; }
    core::TimelineTime duration() const noexcept { return duration_; }
    core::TimelineTime timeOffset() const noexcept { return timeOffset_; }
    core::TimelineTime trimStart() const noexcept { return trimStart_; }
    core::TimelineTime trimEnd() const noexcept { return trimEnd_; }
    double blurIntensity() const noexcept { return blurIntensity_; }

private:
    std::string mediaId_;
    std::string url_;
    std::string mediaType_;
    core::TimelineTime duration_;
    core::TimelineTime timeOffset_;
    core::TimelineTime trimStart_;
    core::TimelineTime trimEnd_;
    double blurIntensity_{15.0};
};

class EffectLayerNode : public SceneNode {
public:
    EffectLayerNode(
        std::string effectType,
        nlohmann::json effectParams,
        core::TimelineTime timeOffset,
        core::TimelineTime duration
    ) : effectType_(std::move(effectType)),
        effectParams_(std::move(effectParams)),
        timeOffset_(timeOffset),
        duration_(duration) {}

    SceneNodeType type() const override { return SceneNodeType::EffectLayer; }

    const std::string& effectType() const noexcept { return effectType_; }
    const nlohmann::json& effectParams() const noexcept { return effectParams_; }
    core::TimelineTime timeOffset() const noexcept { return timeOffset_; }
    core::TimelineTime duration() const noexcept { return duration_; }

private:
    std::string effectType_;
    nlohmann::json effectParams_;
    core::TimelineTime timeOffset_;
    core::TimelineTime duration_;
};

class VisualNode : public SceneNode {
public:
    VisualNode(
        std::string elementId,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        core::TimelineTime trimStart,
        core::TimelineTime trimEnd,
        Transform2D transform,
        double opacity = 1.0,
        std::string blendMode = "normal",
        std::vector<editor::AnimationChannel> animations = {},
        nlohmann::json effects = nlohmann::json::array(),
        nlohmann::json masks = nlohmann::json::array()
    ) : elementId_(std::move(elementId)),
        duration_(duration),
        timeOffset_(timeOffset),
        trimStart_(trimStart),
        trimEnd_(trimEnd),
        transform_(transform),
        opacity_(opacity),
        blendMode_(std::move(blendMode)),
        animations_(std::move(animations)),
        effects_(std::move(effects)),
        masks_(std::move(masks)) {}

    const std::string& elementId() const noexcept { return elementId_; }
    core::TimelineTime duration() const noexcept { return duration_; }
    core::TimelineTime timeOffset() const noexcept { return timeOffset_; }
    core::TimelineTime trimStart() const noexcept { return trimStart_; }
    core::TimelineTime trimEnd() const noexcept { return trimEnd_; }
    const Transform2D& transform() const noexcept { return transform_; }
    double opacity() const noexcept { return opacity_; }
    const std::string& blendMode() const noexcept { return blendMode_; }
    const std::vector<editor::AnimationChannel>& animations() const noexcept { return animations_; }
    const nlohmann::json& effects() const noexcept { return effects_; }
    const nlohmann::json& masks() const noexcept { return masks_; }

protected:
    std::string elementId_;
    core::TimelineTime duration_;
    core::TimelineTime timeOffset_;
    core::TimelineTime trimStart_;
    core::TimelineTime trimEnd_;
    Transform2D transform_;
    double opacity_{1.0};
    std::string blendMode_{"normal"};
    std::vector<editor::AnimationChannel> animations_;
    nlohmann::json effects_;
    nlohmann::json masks_;
};

class VideoNode : public VisualNode {
public:
    VideoNode(
        std::string elementId,
        std::string mediaId,
        std::string url,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        core::TimelineTime trimStart,
        core::TimelineTime trimEnd,
        Transform2D transform,
        double opacity = 1.0,
        std::string blendMode = "normal",
        std::vector<editor::AnimationChannel> animations = {},
        nlohmann::json effects = nlohmann::json::array(),
        nlohmann::json masks = nlohmann::json::array(),
        double retimeRate = 1.0
    ) : VisualNode(
            std::move(elementId),
            duration,
            timeOffset,
            trimStart,
            trimEnd,
            transform,
            opacity,
            std::move(blendMode),
            std::move(animations),
            std::move(effects),
            std::move(masks)
        ),
        mediaId_(std::move(mediaId)),
        url_(std::move(url)),
        retimeRate_(retimeRate) {}

    SceneNodeType type() const override { return SceneNodeType::Video; }

    const std::string& mediaId() const noexcept { return mediaId_; }
    const std::string& url() const noexcept { return url_; }
    double retimeRate() const noexcept { return retimeRate_; }

private:
    std::string mediaId_;
    std::string url_;
    double retimeRate_{1.0};
};

class ImageNode : public VisualNode {
public:
    ImageNode(
        std::string elementId,
        std::string mediaId,
        std::string url,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        core::TimelineTime trimStart,
        core::TimelineTime trimEnd,
        Transform2D transform,
        double opacity = 1.0,
        std::string blendMode = "normal",
        std::vector<editor::AnimationChannel> animations = {},
        nlohmann::json effects = nlohmann::json::array(),
        nlohmann::json masks = nlohmann::json::array(),
        double maxSourceSize = 2048.0
    ) : VisualNode(
            std::move(elementId),
            duration,
            timeOffset,
            trimStart,
            trimEnd,
            transform,
            opacity,
            std::move(blendMode),
            std::move(animations),
            std::move(effects),
            std::move(masks)
        ),
        mediaId_(std::move(mediaId)),
        url_(std::move(url)),
        maxSourceSize_(maxSourceSize) {}

    SceneNodeType type() const override { return SceneNodeType::Image; }

    const std::string& mediaId() const noexcept { return mediaId_; }
    const std::string& url() const noexcept { return url_; }
    double maxSourceSize() const noexcept { return maxSourceSize_; }

private:
    std::string mediaId_;
    std::string url_;
    double maxSourceSize_{2048.0};
};

class TextNode : public SceneNode {
public:
    TextNode(
        std::string elementId,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        std::string content,
        std::string fontFamily = "Arial",
        double fontSize = 15.0,
        std::string fontWeight = "normal",
        std::string fontStyle = "normal",
        std::string textAlign = "center",
        std::string textDecoration = "none",
        double letterSpacing = 0.0,
        double lineHeight = 1.2,
        std::string textColor = "#ffffff",
        std::string backgroundColor = "",
        bool backgroundEnabled = false,
        Transform2D transform = {},
        double opacity = 1.0,
        std::string blendMode = "normal",
        std::vector<editor::AnimationChannel> animations = {},
        nlohmann::json effects = nlohmann::json::array()
    ) : elementId_(std::move(elementId)),
        duration_(duration),
        timeOffset_(timeOffset),
        content_(std::move(content)),
        fontFamily_(std::move(fontFamily)),
        fontSize_(fontSize),
        fontWeight_(std::move(fontWeight)),
        fontStyle_(std::move(fontStyle)),
        textAlign_(std::move(textAlign)),
        textDecoration_(std::move(textDecoration)),
        letterSpacing_(letterSpacing),
        lineHeight_(lineHeight),
        textColor_(std::move(textColor)),
        backgroundColor_(std::move(backgroundColor)),
        backgroundEnabled_(backgroundEnabled),
        transform_(transform),
        opacity_(opacity),
        blendMode_(std::move(blendMode)),
        animations_(std::move(animations)),
        effects_(std::move(effects)) {}

    SceneNodeType type() const override { return SceneNodeType::Text; }

    const std::string& elementId() const noexcept { return elementId_; }
    core::TimelineTime duration() const noexcept { return duration_; }
    core::TimelineTime timeOffset() const noexcept { return timeOffset_; }
    const std::string& content() const noexcept { return content_; }
    const std::string& fontFamily() const noexcept { return fontFamily_; }
    double fontSize() const noexcept { return fontSize_; }
    const std::string& fontWeight() const noexcept { return fontWeight_; }
    const std::string& fontStyle() const noexcept { return fontStyle_; }
    const std::string& textAlign() const noexcept { return textAlign_; }
    const std::string& textDecoration() const noexcept { return textDecoration_; }
    double letterSpacing() const noexcept { return letterSpacing_; }
    double lineHeight() const noexcept { return lineHeight_; }
    const std::string& textColor() const noexcept { return textColor_; }
    const std::string& backgroundColor() const noexcept { return backgroundColor_; }
    bool backgroundEnabled() const noexcept { return backgroundEnabled_; }
    const Transform2D& transform() const noexcept { return transform_; }
    double opacity() const noexcept { return opacity_; }
    const std::string& blendMode() const noexcept { return blendMode_; }
    const std::vector<editor::AnimationChannel>& animations() const noexcept { return animations_; }
    const nlohmann::json& effects() const noexcept { return effects_; }

private:
    std::string elementId_;
    core::TimelineTime duration_;
    core::TimelineTime timeOffset_;
    std::string content_;
    std::string fontFamily_{"Arial"};
    double fontSize_{15.0};
    std::string fontWeight_{"normal"};
    std::string fontStyle_{"normal"};
    std::string textAlign_{"center"};
    std::string textDecoration_{"none"};
    double letterSpacing_{0.0};
    double lineHeight_{1.2};
    std::string textColor_{"#ffffff"};
    std::string backgroundColor_{""};
    bool backgroundEnabled_{false};
    Transform2D transform_;
    double opacity_{1.0};
    std::string blendMode_{"normal"};
    std::vector<editor::AnimationChannel> animations_;
    nlohmann::json effects_;
};

class StickerNode : public VisualNode {
public:
    StickerNode(
        std::string elementId,
        std::string stickerId,
        double intrinsicWidth,
        double intrinsicHeight,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        core::TimelineTime trimStart,
        core::TimelineTime trimEnd,
        Transform2D transform,
        double opacity = 1.0,
        std::string blendMode = "normal",
        std::vector<editor::AnimationChannel> animations = {},
        nlohmann::json effects = nlohmann::json::array()
    ) : VisualNode(
            std::move(elementId),
            duration,
            timeOffset,
            trimStart,
            trimEnd,
            transform,
            opacity,
            std::move(blendMode),
            std::move(animations),
            std::move(effects)
        ),
        stickerId_(std::move(stickerId)),
        intrinsicWidth_(intrinsicWidth),
        intrinsicHeight_(intrinsicHeight) {}

    SceneNodeType type() const override { return SceneNodeType::Sticker; }

    const std::string& stickerId() const noexcept { return stickerId_; }
    double intrinsicWidth() const noexcept { return intrinsicWidth_; }
    double intrinsicHeight() const noexcept { return intrinsicHeight_; }

private:
    std::string stickerId_;
    double intrinsicWidth_{256.0};
    double intrinsicHeight_{256.0};
};

class GraphicNode : public VisualNode {
public:
    GraphicNode(
        std::string elementId,
        std::string definitionId,
        nlohmann::json params,
        core::TimelineTime duration,
        core::TimelineTime timeOffset,
        core::TimelineTime trimStart,
        core::TimelineTime trimEnd,
        Transform2D transform,
        double opacity = 1.0,
        std::string blendMode = "normal",
        std::vector<editor::AnimationChannel> animations = {},
        nlohmann::json effects = nlohmann::json::array(),
        nlohmann::json masks = nlohmann::json::array()
    ) : VisualNode(
            std::move(elementId),
            duration,
            timeOffset,
            trimStart,
            trimEnd,
            transform,
            opacity,
            std::move(blendMode),
            std::move(animations),
            std::move(effects),
            std::move(masks)
        ),
        definitionId_(std::move(definitionId)),
        params_(std::move(params)) {}

    SceneNodeType type() const override { return SceneNodeType::Graphic; }

    const std::string& definitionId() const noexcept { return definitionId_; }
    const nlohmann::json& params() const noexcept { return params_; }

private:
    std::string definitionId_;
    nlohmann::json params_;
};

} // namespace catchim::render
