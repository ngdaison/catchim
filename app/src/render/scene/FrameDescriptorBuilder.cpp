#include "render/scene/FrameDescriptorBuilder.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

QuadTransformDescriptor FrameDescriptorBuilder::fullCanvasTransform(
    int canvasWidth,
    int canvasHeight
) noexcept {
    return QuadTransformDescriptor{
        .centerX = static_cast<double>(canvasWidth) / 2.0,
        .centerY = static_cast<double>(canvasHeight) / 2.0,
        .width = static_cast<double>(canvasWidth),
        .height = static_cast<double>(canvasHeight),
        .rotationDegrees = 0.0,
        .flipX = false,
        .flipY = false
    };
}

QuadTransformDescriptor FrameDescriptorBuilder::computeVisualTransform(
    const Transform2D& transform,
    double sourceWidth,
    double sourceHeight,
    int canvasWidth,
    int canvasHeight
) noexcept {
    if (sourceWidth <= 0.0 || sourceHeight <= 0.0) {
        return fullCanvasTransform(canvasWidth, canvasHeight);
    }
    double containScale = std::min(
        static_cast<double>(canvasWidth) / sourceWidth,
        static_cast<double>(canvasHeight) / sourceHeight
    );
    double scaledWidth = sourceWidth * containScale * transform.scaleX;
    double scaledHeight = sourceHeight * containScale * transform.scaleY;
    double absWidth = std::abs(scaledWidth);
    double absHeight = std::abs(scaledHeight);

    return QuadTransformDescriptor{
        .centerX = static_cast<double>(canvasWidth) / 2.0 + transform.positionX,
        .centerY = static_cast<double>(canvasHeight) / 2.0 + transform.positionY,
        .width = absWidth,
        .height = absHeight,
        .rotationDegrees = transform.rotate,
        .flipX = (scaledWidth < 0.0),
        .flipY = (scaledHeight < 0.0)
    };
}

std::string FrameDescriptorBuilder::transformHash(const QuadTransformDescriptor& t) {
    return std::to_string(t.centerX) + ":" + std::to_string(t.centerY) + ":" +
           std::to_string(t.width) + ":" + std::to_string(t.height) + ":" +
           std::to_string(t.rotationDegrees) + ":" + (t.flipX ? "1" : "0") + ":" +
           (t.flipY ? "1" : "0");
}

FrameDescriptor FrameDescriptorBuilder::buildFrameDescriptor(
    const std::shared_ptr<RootNode>& root,
    core::TimelineTime time,
    int canvasWidth,
    int canvasHeight
) {
    FrameDescriptor frameDesc;
    frameDesc.width = canvasWidth;
    frameDesc.height = canvasHeight;
    frameDesc.clearColor = {0.0, 0.0, 0.0, 1.0};

    if (!root) {
        return frameDesc;
    }

    size_t nodeIndex = 0;
    for (const auto& child : root->children()) {
        std::string path = "root:" + std::to_string(nodeIndex++);

        if (child->type() == SceneNodeType::Color) {
            const auto& cn = static_cast<const ColorNode&>(*child);
            std::string textureId = path + ":color";
            std::string hash = "color:" + cn.color() + ":" + std::to_string(canvasWidth) + "x" + std::to_string(canvasHeight);

            frameDesc.textures.push_back(TextureUploadDescriptor{
                .kind = TextureUploadKind::Rendered,
                .id = textureId,
                .contentHash = hash,
                .width = canvasWidth,
                .height = canvasHeight
            });

            frameDesc.items.push_back(FrameItemDescriptor{
                .type = FrameItemType::Layer,
                .textureId = textureId,
                .transform = fullCanvasTransform(canvasWidth, canvasHeight),
                .opacity = 1.0,
                .blendMode = "normal",
                .effectPassGroups = {},
                .mask = std::nullopt
            });
            continue;
        }

        if (child->type() == SceneNodeType::BlurBackground) {
            const auto& bn = static_cast<const BlurBackgroundNode&>(*child);
            auto res = SceneTreeResolver::resolveBlurBackgroundNode(
                bn,
                time,
                static_cast<double>(canvasWidth),
                static_cast<double>(canvasHeight)
            );
            if (!res) {
                continue;
            }

            std::string textureId = path + ":blur-background";
            std::string hash = "blur:" + bn.mediaId() + ":" + std::to_string(canvasWidth) + "x" + std::to_string(canvasHeight);

            frameDesc.textures.push_back(TextureUploadDescriptor{
                .kind = TextureUploadKind::Rendered,
                .id = textureId,
                .contentHash = hash,
                .width = canvasWidth,
                .height = canvasHeight
            });

            frameDesc.items.push_back(FrameItemDescriptor{
                .type = FrameItemType::Layer,
                .textureId = textureId,
                .transform = fullCanvasTransform(canvasWidth, canvasHeight),
                .opacity = 1.0,
                .blendMode = "normal",
                .effectPassGroups = {res->passes},
                .mask = std::nullopt
            });
            continue;
        }

        if (child->type() == SceneNodeType::EffectLayer) {
            const auto& en = static_cast<const EffectLayerNode&>(*child);
            auto res = SceneTreeResolver::resolveEffectLayerNode(
                en,
                time,
                static_cast<double>(canvasWidth),
                static_cast<double>(canvasHeight)
            );
            if (!res || res->effectPassGroups.empty()) {
                continue;
            }

            frameDesc.items.push_back(FrameItemDescriptor{
                .type = FrameItemType::SceneEffect,
                .textureId = "",
                .transform = {},
                .opacity = 1.0,
                .blendMode = "normal",
                .effectPassGroups = res->effectPassGroups,
                .mask = std::nullopt
            });
            continue;
        }

        if (child->type() == SceneNodeType::Text) {
            const auto& tn = static_cast<const TextNode&>(*child);
            auto res = SceneTreeResolver::resolveTextNode(
                tn,
                time,
                static_cast<double>(canvasWidth),
                static_cast<double>(canvasHeight)
            );
            if (!res) {
                continue;
            }

            std::string textureId = path + ":text";
            std::string hash = "text:" + tn.content() + ":" + std::to_string(canvasWidth) + "x" + std::to_string(canvasHeight);

            frameDesc.textures.push_back(TextureUploadDescriptor{
                .kind = TextureUploadKind::Rendered,
                .id = textureId,
                .contentHash = hash,
                .width = canvasWidth,
                .height = canvasHeight
            });

            frameDesc.items.push_back(FrameItemDescriptor{
                .type = FrameItemType::Layer,
                .textureId = textureId,
                .transform = fullCanvasTransform(canvasWidth, canvasHeight),
                .opacity = res->opacity,
                .blendMode = tn.blendMode(),
                .effectPassGroups = res->effectPassGroups,
                .mask = std::nullopt
            });
            continue;
        }

        if (child->type() == SceneNodeType::Video ||
            child->type() == SceneNodeType::Image ||
            child->type() == SceneNodeType::Sticker ||
            child->type() == SceneNodeType::Graphic) {
            const auto& vn = static_cast<const VisualNode&>(*child);
            auto res = SceneTreeResolver::resolveVisualNode(
                vn,
                time,
                static_cast<double>(canvasWidth),
                static_cast<double>(canvasHeight)
            );
            if (!res) {
                continue;
            }

            double sourceW = 1920.0;
            double sourceH = 1080.0;
            if (child->type() == SceneNodeType::Sticker) {
                const auto& sn = static_cast<const StickerNode&>(*child);
                sourceW = sn.intrinsicWidth();
                sourceH = sn.intrinsicHeight();
            } else if (child->type() == SceneNodeType::Graphic) {
                sourceW = 200.0;
                sourceH = 200.0;
            }

            std::string textureId = path + ":source";
            frameDesc.textures.push_back(TextureUploadDescriptor{
                .kind = TextureUploadKind::External,
                .id = textureId,
                .contentHash = "source:" + vn.elementId(),
                .width = static_cast<int>(sourceW),
                .height = static_cast<int>(sourceH)
            });

            auto quadTransform = computeVisualTransform(
                res->transform,
                sourceW,
                sourceH,
                canvasWidth,
                canvasHeight
            );

            std::optional<LayerMaskDescriptor> maskDesc;
            std::optional<FrameItemDescriptor> strokeLayer;

            if (vn.masks().is_array() && !vn.masks().empty()) {
                const auto& m = vn.masks()[0];
                std::string maskTextureId = path + ":mask";
                double feather = 0.0;
                bool inverted = false;
                double strokeWidth = 0.0;

                if (m.contains("params") && m["params"].is_object()) {
                    feather = m["params"].value("feather", 0.0);
                    inverted = m["params"].value("inverted", false);
                    strokeWidth = m["params"].value("strokeWidth", 0.0);
                }

                maskDesc = LayerMaskDescriptor{
                    .textureId = maskTextureId,
                    .feather = feather,
                    .inverted = inverted
                };

                std::string maskHash = "mask:" + m.value("type", "rectangle") + ":" + transformHash(quadTransform);
                frameDesc.textures.push_back(TextureUploadDescriptor{
                    .kind = TextureUploadKind::Rendered,
                    .id = maskTextureId,
                    .contentHash = maskHash,
                    .width = canvasWidth,
                    .height = canvasHeight
                });

                if (strokeWidth > 0.0) {
                    std::string strokeTextureId = path + ":mask-stroke";
                    std::string strokeHash = "stroke:" + m.value("type", "rectangle") + ":" + transformHash(quadTransform);

                    frameDesc.textures.push_back(TextureUploadDescriptor{
                        .kind = TextureUploadKind::Rendered,
                        .id = strokeTextureId,
                        .contentHash = strokeHash,
                        .width = canvasWidth,
                        .height = canvasHeight
                    });

                    strokeLayer = FrameItemDescriptor{
                        .type = FrameItemType::Layer,
                        .textureId = strokeTextureId,
                        .transform = fullCanvasTransform(canvasWidth, canvasHeight),
                        .opacity = 1.0,
                        .blendMode = "normal",
                        .effectPassGroups = {},
                        .mask = std::nullopt
                    };
                }
            }

            frameDesc.items.push_back(FrameItemDescriptor{
                .type = FrameItemType::Layer,
                .textureId = textureId,
                .transform = quadTransform,
                .opacity = res->opacity,
                .blendMode = vn.blendMode(),
                .effectPassGroups = res->effectPassGroups,
                .mask = maskDesc
            });

            if (strokeLayer) {
                frameDesc.items.push_back(*strokeLayer);
            }
        }
    }

    return frameDesc;
}

} // namespace catchim::render
