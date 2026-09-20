#pragma once

#include "render/scene/SceneNodes.h"
#include "render/scene/SceneTreeResolver.h"
#include "render/canvas/CanvasTransformPipeline.h"
#include <string>
#include <vector>
#include <memory>
#include <array>
#include <optional>

namespace catchim::render {

struct QuadTransformDescriptor {
    double centerX{0.0};
    double centerY{0.0};
    double width{0.0};
    double height{0.0};
    double rotationDegrees{0.0};
    bool flipX{false};
    bool flipY{false};

    bool operator==(const QuadTransformDescriptor& other) const = default;
};

struct LayerMaskDescriptor {
    std::string textureId;
    double feather{0.0};
    bool inverted{false};

    bool operator==(const LayerMaskDescriptor& other) const = default;
};

enum class FrameItemType {
    Layer,
    SceneEffect
};

struct FrameItemDescriptor {
    FrameItemType type{FrameItemType::Layer};
    std::string textureId;
    QuadTransformDescriptor transform;
    double opacity{1.0};
    std::string blendMode{"normal"};
    std::vector<std::vector<EffectPass>> effectPassGroups;
    std::optional<LayerMaskDescriptor> mask{std::nullopt};
};

enum class TextureUploadKind {
    External,
    Rendered
};

struct TextureUploadDescriptor {
    TextureUploadKind kind{TextureUploadKind::Rendered};
    std::string id;
    std::string contentHash;
    int width{0};
    int height{0};
};

struct FrameDescriptor {
    int width{1920};
    int height{1080};
    std::array<double, 4> clearColor{0.0, 0.0, 0.0, 1.0};
    std::vector<FrameItemDescriptor> items;
    std::vector<TextureUploadDescriptor> textures;
};

/**
 * @brief Translates a resolved SceneNode tree into a FrameDescriptor with items and textures.
 * Corresponds to web/src/services/renderer/compositor/frame-descriptor.ts.
 */
class FrameDescriptorBuilder {
public:
    static FrameDescriptor buildFrameDescriptor(
        const std::shared_ptr<RootNode>& root,
        core::TimelineTime time,
        int canvasWidth = 1920,
        int canvasHeight = 1080
    );

    static QuadTransformDescriptor fullCanvasTransform(int canvasWidth, int canvasHeight) noexcept;

    static QuadTransformDescriptor computeVisualTransform(
        const Transform2D& transform,
        double sourceWidth,
        double sourceHeight,
        int canvasWidth,
        int canvasHeight
    ) noexcept;

    static std::string transformHash(const QuadTransformDescriptor& transform);
};

} // namespace catchim::render
