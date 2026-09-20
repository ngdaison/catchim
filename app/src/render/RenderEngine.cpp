#include "RenderEngine.h"

namespace catchim::render {

RenderEngine::RenderEngine(int32_t width, int32_t height)
    : compositor_(width, height)
{
}

void RenderEngine::setCanvasSize(int32_t width, int32_t height) {
    compositor_.setCanvasSize(width, height);
}

const CompositorOutput& RenderEngine::renderFrame(
    const editor::Project& project,
    const media::MediaLibrary& /* mediaLibrary */,
    core::TimelineTime time
) {
    // 1. Clear background
    const auto& bg = project.settings().background;
    uint8_t r = 0, g = 0, b = 0;
    if (bg.type == editor::ProjectBackground::Type::Color && !bg.color.empty()) {
        if (bg.color[0] == '#' && bg.color.size() == 7) {
            try {
                r = static_cast<uint8_t>(std::stoi(bg.color.substr(1, 2), nullptr, 16));
                g = static_cast<uint8_t>(std::stoi(bg.color.substr(3, 2), nullptr, 16));
                b = static_cast<uint8_t>(std::stoi(bg.color.substr(5, 2), nullptr, 16));
            } catch (...) {}
        }
    }
    compositor_.clear(r, g, b, 255);

    const editor::Timeline* tl = project.activeTimeline();
    if (!tl) return compositor_.getOutput();

    // 2. Iterate tracks in reverse order (bottom to top: main -> overlay)
    int32_t zIndex = 0;

    // Main track
    const auto& mainTrack = tl->mainTrack();
    if (!mainTrack.isHidden()) {
        for (const auto& clip : mainTrack.clips()) {
            if (time >= clip.startTime() && time < clip.endTime()) {
                RenderLayer layer;
                layer.id = clip.id().str();
                layer.zIndex = zIndex++;
                layer.sourceWidth = compositor_.width();
                layer.sourceHeight = compositor_.height();

                // Extract Transform
                layer.transform.positionX = clip.getParam<double>("transform.positionX", 0.0);
                layer.transform.positionY = clip.getParam<double>("transform.positionY", 0.0);
                layer.transform.scaleX = clip.getParam<double>("transform.scaleX", 1.0);
                layer.transform.scaleY = clip.getParam<double>("transform.scaleY", 1.0);
                layer.transform.rotate = clip.getParam<double>("transform.rotate", 0.0);
                layer.transform.opacity = clip.getParam<double>("opacity", 1.0);

                // Default placeholder color (dark gray with blue tint for video clip)
                layer.rgbaPixels.resize(layer.sourceWidth * layer.sourceHeight * 4);
                for (size_t i = 0; i < layer.rgbaPixels.size(); i += 4) {
                    layer.rgbaPixels[i + 0] = 35;
                    layer.rgbaPixels[i + 1] = 45;
                    layer.rgbaPixels[i + 2] = 65;
                    layer.rgbaPixels[i + 3] = 255;
                }

                compositor_.compositeLayer(layer);
            }
        }
    }

    // Overlay tracks
    for (const auto& track : tl->overlayTracks()) {
        if (track.isHidden()) continue;
        for (const auto& clip : track.clips()) {
            if (time >= clip.startTime() && time < clip.endTime()) {
                RenderLayer layer;
                layer.id = clip.id().str();
                layer.zIndex = zIndex++;
                layer.sourceWidth = 600;
                layer.sourceHeight = 200;

                layer.transform.positionX = clip.getParam<double>("transform.positionX", 0.0);
                layer.transform.positionY = clip.getParam<double>("transform.positionY", 0.0);
                layer.transform.scaleX = clip.getParam<double>("transform.scaleX", 1.0);
                layer.transform.scaleY = clip.getParam<double>("transform.scaleY", 1.0);
                layer.transform.rotate = clip.getParam<double>("transform.rotate", 0.0);
                layer.transform.opacity = clip.getParam<double>("opacity", 1.0);

                // Text / Graphic color
                layer.rgbaPixels.resize(layer.sourceWidth * layer.sourceHeight * 4);
                uint8_t tr = 93, tg = 186, tb = 160; // #5DBAA0
                for (size_t i = 0; i < layer.rgbaPixels.size(); i += 4) {
                    layer.rgbaPixels[i + 0] = tr;
                    layer.rgbaPixels[i + 1] = tg;
                    layer.rgbaPixels[i + 2] = tb;
                    layer.rgbaPixels[i + 3] = 200;
                }

                compositor_.compositeLayer(layer);
            }
        }
    }

    return compositor_.getOutput();
}

} // namespace catchim::render
