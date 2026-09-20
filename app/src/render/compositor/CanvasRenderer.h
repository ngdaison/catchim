#pragma once

#include "render/compositor/RenderSurface.h"
#include "render/compositor/TextureCacheManager.h"
#include "render/scene/SceneNodes.h"
#include "render/scene/FrameDescriptorBuilder.h"
#include "core/time/TimelineTime.h"
#include <memory>

namespace catchim::render {

struct CanvasRendererParams {
    int width{1920};
    int height{1080};
    core::FrameRate fps{30, 1};
};

/**
 * @brief Coordinates the end-to-end rendering and compositing pipeline for a frame.
 * Corresponds to web/src/services/renderer/canvas-renderer.ts.
 */
class CanvasRenderer {
public:
    explicit CanvasRenderer(const CanvasRendererParams& params = {});

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    core::FrameRate fps() const noexcept { return fps_; }

    void setSize(int width, int height);

    void render(
        const std::shared_ptr<RootNode>& root,
        core::TimelineTime time,
        RenderSurface& targetSurface
    );

    TextureCacheManager& textureCache() noexcept { return textureCache_; }
    const TextureCacheManager& textureCache() const noexcept { return textureCache_; }

private:
    int width_{1920};
    int height_{1080};
    core::FrameRate fps_{30, 1};
    TextureCacheManager textureCache_;
};

} // namespace catchim::render
