#include "render/MaskEngine.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

std::vector<uint8_t> MaskEngine::generateMask(
    const MaskDefinition& mask,
    int32_t width,
    int32_t height
) {
    if (width <= 0 || height <= 0) return {};
    std::vector<uint8_t> buffer(width * height, 0);

    double cx = mask.centerX * width;
    double cy = mask.centerY * height;
    double hw = (mask.width * width) / 2.0;
    double hh = (mask.height * height) / 2.0;
    double feather = std::max(0.001, mask.feather);

    for (int32_t y = 0; y < height; ++y) {
        for (int32_t x = 0; x < width; ++x) {
            double alpha = 0.0;

            if (mask.shape == MaskShape::Ellipse) {
                if (hw > 0.001 && hh > 0.001) {
                    double dx = (x - cx) / hw;
                    double dy = (y - cy) / hh;
                    double dist = std::sqrt(dx * dx + dy * dy);

                    if (mask.feather > 0.001) {
                        double featherRatio = mask.feather / std::max(1.0, std::min(hw, hh));
                        if (dist <= 1.0 - featherRatio) {
                            alpha = 1.0;
                        } else if (dist >= 1.0 + featherRatio) {
                            alpha = 0.0;
                        } else {
                            alpha = 0.5 * (1.0 - (dist - 1.0) / featherRatio);
                        }
                    } else {
                        alpha = (dist <= 1.0) ? 1.0 : 0.0;
                    }
                }
            } else { // Rectangle (Default)
                double left = cx - hw;
                double right = cx + hw;
                double top = cy - hh;
                double bottom = cy + hh;

                if (mask.feather > 0.001) {
                    double distLeft = x - left;
                    double distRight = right - x;
                    double distTop = y - top;
                    double distBottom = bottom - y;
                    double minDist = std::min({distLeft, distRight, distTop, distBottom});

                    if (minDist <= -feather) {
                        alpha = 0.0;
                    } else if (minDist >= feather) {
                        alpha = 1.0;
                    } else {
                        alpha = (minDist + feather) / (2.0 * feather);
                    }
                } else {
                    alpha = (x >= left && x <= right && y >= top && y <= bottom) ? 1.0 : 0.0;
                }
            }

            alpha = std::clamp(alpha, 0.0, 1.0);
            if (mask.inverted) {
                alpha = 1.0 - alpha;
            }

            buffer[y * width + x] = static_cast<uint8_t>(alpha * 255.0);
        }
    }

    return buffer;
}

void MaskEngine::applyMaskToLayer(
    uint8_t* rgbaPixels,
    const uint8_t* alphaMask,
    int32_t width,
    int32_t height
) {
    if (!rgbaPixels || !alphaMask || width <= 0 || height <= 0) return;
    size_t total = static_cast<size_t>(width * height);
    for (size_t i = 0; i < total; ++i) {
        uint8_t maskVal = alphaMask[i];
        rgbaPixels[i * 4 + 3] = static_cast<uint8_t>((rgbaPixels[i * 4 + 3] * maskVal) / 255);
    }
}

} // namespace catchim::render
