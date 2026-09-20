#include "render/TransitionEngine.h"
#include <cmath>

namespace catchim::render {

void TransitionEngine::blend(
    uint8_t* dst,
    const uint8_t* frameA,
    const uint8_t* frameB,
    int32_t w,
    int32_t h,
    TransitionType type,
    double progress
) noexcept {
    if (!dst || !frameA || !frameB || w <= 0 || h <= 0) return;
    double t = std::clamp(progress, 0.0, 1.0);
    size_t totalPixels = static_cast<size_t>(w) * h;

    if (t <= 0.0) {
        for (size_t i = 0; i < totalPixels * 4; ++i) dst[i] = frameA[i];
        return;
    }
    if (t >= 1.0) {
        for (size_t i = 0; i < totalPixels * 4; ++i) dst[i] = frameB[i];
        return;
    }

    switch (type) {
        case TransitionType::Crossfade: {
            float tF = static_cast<float>(t);
            float invTF = 1.0f - tF;
            for (size_t i = 0; i < totalPixels; ++i) {
                dst[i * 4 + 0] = static_cast<uint8_t>(frameA[i * 4 + 0] * invTF + frameB[i * 4 + 0] * tF);
                dst[i * 4 + 1] = static_cast<uint8_t>(frameA[i * 4 + 1] * invTF + frameB[i * 4 + 1] * tF);
                dst[i * 4 + 2] = static_cast<uint8_t>(frameA[i * 4 + 2] * invTF + frameB[i * 4 + 2] * tF);
                dst[i * 4 + 3] = 255;
            }
            break;
        }

        case TransitionType::FadeToBlack: {
            if (t < 0.5) {
                float factor = static_cast<float>(1.0 - t * 2.0);
                for (size_t i = 0; i < totalPixels; ++i) {
                    dst[i * 4 + 0] = static_cast<uint8_t>(frameA[i * 4 + 0] * factor);
                    dst[i * 4 + 1] = static_cast<uint8_t>(frameA[i * 4 + 1] * factor);
                    dst[i * 4 + 2] = static_cast<uint8_t>(frameA[i * 4 + 2] * factor);
                    dst[i * 4 + 3] = 255;
                }
            } else {
                float factor = static_cast<float>((t - 0.5) * 2.0);
                for (size_t i = 0; i < totalPixels; ++i) {
                    dst[i * 4 + 0] = static_cast<uint8_t>(frameB[i * 4 + 0] * factor);
                    dst[i * 4 + 1] = static_cast<uint8_t>(frameB[i * 4 + 1] * factor);
                    dst[i * 4 + 2] = static_cast<uint8_t>(frameB[i * 4 + 2] * factor);
                    dst[i * 4 + 3] = 255;
                }
            }
            break;
        }

        case TransitionType::FadeToWhite: {
            if (t < 0.5) {
                float factor = static_cast<float>(t * 2.0);
                float invFactor = 1.0f - factor;
                for (size_t i = 0; i < totalPixels; ++i) {
                    dst[i * 4 + 0] = static_cast<uint8_t>(frameA[i * 4 + 0] * invFactor + 255.0f * factor);
                    dst[i * 4 + 1] = static_cast<uint8_t>(frameA[i * 4 + 1] * invFactor + 255.0f * factor);
                    dst[i * 4 + 2] = static_cast<uint8_t>(frameA[i * 4 + 2] * invFactor + 255.0f * factor);
                    dst[i * 4 + 3] = 255;
                }
            } else {
                float factor = static_cast<float>((t - 0.5) * 2.0);
                float invFactor = 1.0f - factor;
                for (size_t i = 0; i < totalPixels; ++i) {
                    dst[i * 4 + 0] = static_cast<uint8_t>(255.0f * invFactor + frameB[i * 4 + 0] * factor);
                    dst[i * 4 + 1] = static_cast<uint8_t>(255.0f * invFactor + frameB[i * 4 + 1] * factor);
                    dst[i * 4 + 2] = static_cast<uint8_t>(255.0f * invFactor + frameB[i * 4 + 2] * factor);
                    dst[i * 4 + 3] = 255;
                }
            }
            break;
        }

        case TransitionType::WipeLeft: {
            int32_t splitX = static_cast<int32_t>(w * t);
            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    size_t idx = (static_cast<size_t>(y) * w + x) * 4;
                    const uint8_t* src = (x < splitX) ? frameB : frameA;
                    dst[idx + 0] = src[idx + 0];
                    dst[idx + 1] = src[idx + 1];
                    dst[idx + 2] = src[idx + 2];
                    dst[idx + 3] = 255;
                }
            }
            break;
        }

        case TransitionType::WipeRight: {
            int32_t splitX = static_cast<int32_t>(w * (1.0 - t));
            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    size_t idx = (static_cast<size_t>(y) * w + x) * 4;
                    const uint8_t* src = (x >= splitX) ? frameB : frameA;
                    dst[idx + 0] = src[idx + 0];
                    dst[idx + 1] = src[idx + 1];
                    dst[idx + 2] = src[idx + 2];
                    dst[idx + 3] = 255;
                }
            }
            break;
        }

        case TransitionType::SlideLeft: {
            int32_t shift = static_cast<int32_t>(w * t);
            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    size_t dstIdx = (static_cast<size_t>(y) * w + x) * 4;
                    if (x < w - shift) {
                        size_t srcIdx = (static_cast<size_t>(y) * w + (x + shift)) * 4;
                        dst[dstIdx + 0] = frameA[srcIdx + 0];
                        dst[dstIdx + 1] = frameA[srcIdx + 1];
                        dst[dstIdx + 2] = frameA[srcIdx + 2];
                    } else {
                        size_t srcIdx = (static_cast<size_t>(y) * w + (x - (w - shift))) * 4;
                        dst[dstIdx + 0] = frameB[srcIdx + 0];
                        dst[dstIdx + 1] = frameB[srcIdx + 1];
                        dst[dstIdx + 2] = frameB[srcIdx + 2];
                    }
                    dst[dstIdx + 3] = 255;
                }
            }
            break;
        }

        case TransitionType::SlideRight: {
            int32_t shift = static_cast<int32_t>(w * t);
            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    size_t dstIdx = (static_cast<size_t>(y) * w + x) * 4;
                    if (x < shift) {
                        size_t srcIdx = (static_cast<size_t>(y) * w + (x + (w - shift))) * 4;
                        dst[dstIdx + 0] = frameB[srcIdx + 0];
                        dst[dstIdx + 1] = frameB[srcIdx + 1];
                        dst[dstIdx + 2] = frameB[srcIdx + 2];
                    } else {
                        size_t srcIdx = (static_cast<size_t>(y) * w + (x - shift)) * 4;
                        dst[dstIdx + 0] = frameA[srcIdx + 0];
                        dst[dstIdx + 1] = frameA[srcIdx + 1];
                        dst[dstIdx + 2] = frameA[srcIdx + 2];
                    }
                    dst[dstIdx + 3] = 255;
                }
            }
            break;
        }

        case TransitionType::ZoomIn: {
            float tF = static_cast<float>(t);
            float scale = std::max(0.01f, tF);
            double centerX = w / 2.0;
            double centerY = h / 2.0;

            for (int32_t y = 0; y < h; ++y) {
                for (int32_t x = 0; x < w; ++x) {
                    size_t dstIdx = (static_cast<size_t>(y) * w + x) * 4;

                    double srcBX = (x - centerX) / scale + centerX;
                    double srcBY = (y - centerY) / scale + centerY;

                    if (srcBX >= 0 && srcBX < w && srcBY >= 0 && srcBY < h) {
                        int32_t bx = static_cast<int32_t>(srcBX);
                        int32_t by = static_cast<int32_t>(srcBY);
                        size_t bIdx = (static_cast<size_t>(by) * w + bx) * 4;
                        // Alpha blend between frameA and scaled frameB
                        dst[dstIdx + 0] = static_cast<uint8_t>(frameA[dstIdx + 0] * (1.0f - tF) + frameB[bIdx + 0] * tF);
                        dst[dstIdx + 1] = static_cast<uint8_t>(frameA[dstIdx + 1] * (1.0f - tF) + frameB[bIdx + 1] * tF);
                        dst[dstIdx + 2] = static_cast<uint8_t>(frameA[dstIdx + 2] * (1.0f - tF) + frameB[bIdx + 2] * tF);
                    } else {
                        dst[dstIdx + 0] = frameA[dstIdx + 0];
                        dst[dstIdx + 1] = frameA[dstIdx + 1];
                        dst[dstIdx + 2] = frameA[dstIdx + 2];
                    }
                    dst[dstIdx + 3] = 255;
                }
            }
            break;
        }
    }
}

} // namespace catchim::render
