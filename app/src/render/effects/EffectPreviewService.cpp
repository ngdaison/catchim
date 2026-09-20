#include "render/effects/EffectPreviewService.h"
#include <cmath>
#include <algorithm>

namespace catchim::render {

EffectPreviewService::EffectPreviewService() {
    getTestSource(PREVIEW_SIZE, PREVIEW_SIZE);
}

std::shared_ptr<RenderSurface> EffectPreviewService::getTestSource(int width, int height) {
    if (testSource_ && testSource_->width() == width && testSource_->height() == height) {
        return testSource_;
    }

    testSource_ = std::make_shared<RenderSurface>(width, height);

    // Generate a vibrant synthetic test pattern with gradients and shapes
    double centerX = width / 2.0;
    double centerY = height / 2.0;
    double maxRadius = std::min(width, height) / 3.0;

    for (int y = 0; y < height; ++y) {
        double ny = static_cast<double>(y) / height;
        for (int x = 0; x < width; ++x) {
            double nx = static_cast<double>(x) / width;

            // Gradient base
            uint8_t r = static_cast<uint8_t>(std::clamp(nx * 255.0, 0.0, 255.0));
            uint8_t g = static_cast<uint8_t>(std::clamp(ny * 255.0, 0.0, 255.0));
            uint8_t b = static_cast<uint8_t>(std::clamp((1.0 - (nx + ny) * 0.5) * 255.0, 0.0, 255.0));

            // Center circle
            double dx = x - centerX;
            double dy = y - centerY;
            double dist = std::sqrt(dx * dx + dy * dy);
            if (dist < maxRadius) {
                double t = dist / maxRadius;
                r = static_cast<uint8_t>(std::clamp((1.0 - t) * 255.0 + t * r, 0.0, 255.0));
                g = static_cast<uint8_t>(std::clamp((1.0 - t) * 200.0 + t * g, 0.0, 255.0));
                b = static_cast<uint8_t>(std::clamp((1.0 - t) * 50.0 + t * b, 0.0, 255.0));
            }

            testSource_->setPixel(x, y, RenderSurface::makeRgba(r, g, b, 255));
        }
    }

    return testSource_;
}

void EffectPreviewService::renderPreview(
    const std::string& effectType,
    const nlohmann::json& params,
    RenderSurface& targetSurface,
    std::optional<std::pair<int, int>> /*uniformDimensions*/
) {
    targetSurface.resize(PREVIEW_SIZE, PREVIEW_SIZE);
    auto source = getTestSource(PREVIEW_SIZE, PREVIEW_SIZE);
    if (!source) {
        targetSurface.clear(0, 0, 0, 255);
        return;
    }

    targetSurface.copyFrom(*source, 0, 0);

    if (effectType == "blur") {
        double intensity = params.value("intensity", 15.0);
        int radius = std::clamp(static_cast<int>(std::round(intensity * 0.5)), 1, 30);

        RenderSurface temp(PREVIEW_SIZE, PREVIEW_SIZE);
        temp.copyFrom(targetSurface);

        for (int y = 0; y < PREVIEW_SIZE; ++y) {
            for (int x = 0; x < PREVIEW_SIZE; ++x) {
                uint32_t sumR = 0, sumG = 0, sumB = 0;
                int count = 0;
                for (int ky = -radius; ky <= radius; ++ky) {
                    int sy = std::clamp(y + ky, 0, PREVIEW_SIZE - 1);
                    for (int kx = -radius; kx <= radius; ++kx) {
                        int sx = std::clamp(x + kx, 0, PREVIEW_SIZE - 1);
                        uint32_t p = temp.getPixel(sx, sy);
                        uint8_t r, g, b, a;
                        RenderSurface::unpackRgba(p, r, g, b, a);
                        sumR += r;
                        sumG += g;
                        sumB += b;
                        ++count;
                    }
                }
                if (count > 0) {
                    targetSurface.setPixel(x, y, RenderSurface::makeRgba(
                        static_cast<uint8_t>(sumR / count),
                        static_cast<uint8_t>(sumG / count),
                        static_cast<uint8_t>(sumB / count),
                        255
                    ));
                }
            }
        }
    } else if (effectType == "vignette") {
        double intensity = params.value("intensity", 0.5);
        double radius = params.value("radius", 0.75) * (PREVIEW_SIZE / 2.0);
        double centerX = PREVIEW_SIZE / 2.0;
        double centerY = PREVIEW_SIZE / 2.0;

        for (int y = 0; y < PREVIEW_SIZE; ++y) {
            for (int x = 0; x < PREVIEW_SIZE; ++x) {
                double dist = std::hypot(x - centerX, y - centerY);
                if (dist > radius) {
                    double falloff = std::min(1.0, (dist - radius) / (PREVIEW_SIZE / 2.0));
                    double factor = 1.0 - falloff * intensity;

                    uint32_t p = targetSurface.getPixel(x, y);
                    uint8_t r, g, b, a;
                    RenderSurface::unpackRgba(p, r, g, b, a);
                    r = static_cast<uint8_t>(std::clamp(r * factor, 0.0, 255.0));
                    g = static_cast<uint8_t>(std::clamp(g * factor, 0.0, 255.0));
                    b = static_cast<uint8_t>(std::clamp(b * factor, 0.0, 255.0));
                    targetSurface.setPixel(x, y, RenderSurface::makeRgba(r, g, b, a));
                }
            }
        }
    } else if (effectType == "grayscale") {
        for (int y = 0; y < PREVIEW_SIZE; ++y) {
            for (int x = 0; x < PREVIEW_SIZE; ++x) {
                uint32_t p = targetSurface.getPixel(x, y);
                uint8_t r, g, b, a;
                RenderSurface::unpackRgba(p, r, g, b, a);
                uint8_t gray = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
                targetSurface.setPixel(x, y, RenderSurface::makeRgba(gray, gray, gray, a));
            }
        }
    } else if (effectType == "invert") {
        for (int y = 0; y < PREVIEW_SIZE; ++y) {
            for (int x = 0; x < PREVIEW_SIZE; ++x) {
                uint32_t p = targetSurface.getPixel(x, y);
                uint8_t r, g, b, a;
                RenderSurface::unpackRgba(p, r, g, b, a);
                targetSurface.setPixel(x, y, RenderSurface::makeRgba(255 - r, 255 - g, 255 - b, a));
            }
        }
    }
}

} // namespace catchim::render
