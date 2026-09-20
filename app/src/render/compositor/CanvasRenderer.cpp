#include "render/compositor/CanvasRenderer.h"
#include "render/diagnostics/RenderPerformanceProfiler.h"

namespace catchim::render {

CanvasRenderer::CanvasRenderer(const CanvasRendererParams& params)
    : width_(params.width),
      height_(params.height),
      fps_(params.fps) {}

void CanvasRenderer::setSize(int width, int height) {
    if (width > 0) width_ = width;
    if (height > 0) height_ = height;
}

void CanvasRenderer::render(
    const std::shared_ptr<RootNode>& root,
    core::TimelineTime time,
    RenderSurface& targetSurface
) {
    if (targetSurface.width() != width_ || targetSurface.height() != height_) {
        targetSurface.resize(width_, height_);
    }

    auto& profiler = RenderPerformanceProfiler::instance();

    FrameDescriptor frameDesc;
    profiler.measureSpan("buildFrame", [&]() {
        frameDesc = FrameDescriptorBuilder::buildFrameDescriptor(root, time, width_, height_);
    });

    profiler.measureSpan("syncTextures", [&]() {
        textureCache_.syncTextures(frameDesc.textures);
    });

    profiler.measureSpan("renderFrame", [&]() {
        targetSurface.clear(frameDesc.clearColor);

        for (const auto& item : frameDesc.items) {
            if (item.type == FrameItemType::Layer) {
                auto texSurface = textureCache_.getTexture(item.textureId);
                if (texSurface) {
                    targetSurface.blendOver(*texSurface, item.transform, item.opacity, item.blendMode);
                }
            } else if (item.type == FrameItemType::SceneEffect) {
                // Scene effect pass groups
            }
        }
    });

    profiler.onFrameComplete();
}

} // namespace catchim::render
