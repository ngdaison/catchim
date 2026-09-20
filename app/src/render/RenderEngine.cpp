#include "RenderEngine.h"
#include "media/decoder/NativeVideoDecoder.h"
#include "media/MediaLibrary.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

RenderEngine::RenderEngine(int32_t width, int32_t height)
    : compositor_(width, height)
{
}

void RenderEngine::setCanvasSize(int32_t width, int32_t height) {
    compositor_.setCanvasSize(width, height);
}

static void applyEffectToPixels(std::vector<uint8_t>& pixels, int w, int h, const std::string& effectName, double intensity) {
    if (pixels.empty() || effectName.empty() || intensity <= 0.001) return;

    if (effectName.find("Monochrome") != std::string::npos || effectName.find("Đen trắng") != std::string::npos || effectName.find("grayscale") != std::string::npos) {
        for (size_t i = 0; i < pixels.size(); i += 4) {
            uint8_t r = pixels[i + 0];
            uint8_t g = pixels[i + 1];
            uint8_t b = pixels[i + 2];
            uint8_t gray = static_cast<uint8_t>((r * 77 + g * 150 + b * 29) >> 8);
            pixels[i + 0] = static_cast<uint8_t>(r + (gray - r) * intensity);
            pixels[i + 1] = static_cast<uint8_t>(g + (gray - g) * intensity);
            pixels[i + 2] = static_cast<uint8_t>(b + (gray - b) * intensity);
        }
    } else if (effectName.find("Invert") != std::string::npos || effectName.find("Đảo màu") != std::string::npos) {
        for (size_t i = 0; i < pixels.size(); i += 4) {
            uint8_t r = pixels[i + 0];
            uint8_t g = pixels[i + 1];
            uint8_t b = pixels[i + 2];
            pixels[i + 0] = static_cast<uint8_t>(r + ((255 - r) - r) * intensity);
            pixels[i + 1] = static_cast<uint8_t>(g + ((255 - g) - g) * intensity);
            pixels[i + 2] = static_cast<uint8_t>(b + ((255 - b) - b) * intensity);
        }
    } else if (effectName.find("Vignette") != std::string::npos || effectName.find("Tối viền") != std::string::npos) {
        double cx = w / 2.0;
        double cy = h / 2.0;
        double maxDist = std::sqrt(cx * cx + cy * cy);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                size_t i = (y * w + x) * 4;
                double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / maxDist;
                double factor = std::clamp(1.0 - (dist * dist * intensity), 0.0, 1.0);
                pixels[i + 0] = static_cast<uint8_t>(pixels[i + 0] * factor);
                pixels[i + 1] = static_cast<uint8_t>(pixels[i + 1] * factor);
                pixels[i + 2] = static_cast<uint8_t>(pixels[i + 2] * factor);
            }
        }
    }
}

const CompositorOutput& RenderEngine::renderFrame(
    const editor::Project& project,
    const media::MediaLibrary& mediaLibrary,
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

    int32_t zIndex = 0;

    auto renderClipLayer = [&](const editor::Clip& clip) {
        if (time < clip.startTime() || time >= clip.endTime()) return;

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

        bool decoded = false;

        // Try decoding real video or image frame if mediaId is present
        if (!clip.mediaId().isEmpty()) {
            auto asset = mediaLibrary.findAsset(clip.mediaId());
            if (asset) {
                double speed = clip.getParam<double>("speed", 1.0);
                if (speed <= 0.01) speed = 1.0;

                double clipElapsed = (time - clip.startTime()).toSeconds() * speed + clip.trimStart().toSeconds();
                int fw = 0, fh = 0;
                std::vector<uint8_t> framePixels;

                if (media::NativeVideoDecoder::instance().getFrame(asset->filePath(), clipElapsed, fw, fh, framePixels)) {
                    layer.sourceWidth = fw;
                    layer.sourceHeight = fh;
                    layer.rgbaPixels = std::move(framePixels);
                    decoded = true;
                }
            }
        }

        if (!decoded) {
            // Placeholder fallback
            layer.rgbaPixels.resize(layer.sourceWidth * layer.sourceHeight * 4);
            uint8_t pr = 35, pg = 45, pb = 65;
            if (clip.type() == editor::ClipType::Text || clip.type() == editor::ClipType::Graphic) {
                pr = 56; pg = 189; pb = 248;
            }
            for (size_t i = 0; i < layer.rgbaPixels.size(); i += 4) {
                layer.rgbaPixels[i + 0] = pr;
                layer.rgbaPixels[i + 1] = pg;
                layer.rgbaPixels[i + 2] = pb;
                layer.rgbaPixels[i + 3] = 255;
            }
        }

        // Apply real visual effects if configured
        std::string effName = clip.getParam<std::string>("effect.name", "");
        double effIntensity = clip.getParam<double>("effect.intensity", 1.0);
        if (!effName.empty()) {
            applyEffectToPixels(layer.rgbaPixels, layer.sourceWidth, layer.sourceHeight, effName, effIntensity);
        }

        compositor_.compositeLayer(layer);
    };

    // Main track
    const auto& mainTrack = tl->mainTrack();
    if (!mainTrack.isHidden()) {
        for (const auto& clip : mainTrack.clips()) {
            renderClipLayer(clip);
        }
    }

    // Overlay tracks
    for (const auto& track : tl->overlayTracks()) {
        if (track.isHidden()) continue;
        for (const auto& clip : track.clips()) {
            renderClipLayer(clip);
        }
    }

    return compositor_.getOutput();
}

} // namespace catchim::render
