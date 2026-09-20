#pragma once

#include "Compositor.h"
#include "editor/project/Project.h"
#include "media/MediaLibrary.h"
#include "core/time/TimelineTime.h"
#include <memory>

namespace catchim::render {

class RenderEngine {
public:
    explicit RenderEngine(int32_t width = 1920, int32_t height = 1080);

    void setCanvasSize(int32_t width, int32_t height);

    const CompositorOutput& renderFrame(
        const editor::Project& project,
        const media::MediaLibrary& mediaLibrary,
        core::TimelineTime time
    );

    const CompositorOutput& lastRenderedFrame() const noexcept {
        return compositor_.getOutput();
    }

private:
    Compositor compositor_;
};

} // namespace catchim::render
