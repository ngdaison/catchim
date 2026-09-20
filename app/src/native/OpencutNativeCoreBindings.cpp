#include "OpencutNativeCoreBindings.h"
#include "core/time/TimelineTime.h"
#include "core/math/Bezier.h"
#include <cmath>
#include <algorithm>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::native {

int64_t OpencutNativeCoreBindings::ticksPerSecond() noexcept {
    return core::TICKS_PER_SECOND;
}

int64_t OpencutNativeCoreBindings::fromSeconds(double seconds) noexcept {
    return core::TimelineTime::fromSeconds(seconds).ticks();
}

double OpencutNativeCoreBindings::toSeconds(int64_t ticks) noexcept {
    return core::TimelineTime(ticks).toSeconds();
}

int64_t OpencutNativeCoreBindings::roundToFrame(int64_t ticks, int32_t fpsNum, int32_t fpsDen) noexcept {
    core::FrameRate rate{fpsNum, fpsDen};
    return core::TimelineTime(ticks).roundToFrame(rate).ticks();
}

int64_t OpencutNativeCoreBindings::floorToFrame(int64_t ticks, int32_t fpsNum, int32_t fpsDen) noexcept {
    core::FrameRate rate{fpsNum, fpsDen};
    return core::TimelineTime(ticks).floorToFrame(rate).ticks();
}

int64_t OpencutNativeCoreBindings::lastFrame(int64_t durationTicks, int32_t fpsNum, int32_t fpsDen) noexcept {
    core::FrameRate rate{fpsNum, fpsDen};
    return core::TimelineTime::lastFrameTime(core::TimelineTime(durationTicks), rate).ticks();
}

int64_t OpencutNativeCoreBindings::snappedSeek(int64_t timeTicks, int64_t durationTicks, int32_t fpsNum, int32_t fpsDen) noexcept {
    core::FrameRate rate{fpsNum, fpsDen};
    return core::TimelineTime(timeTicks).snappedSeek(core::TimelineTime(durationTicks), rate).ticks();
}

double OpencutNativeCoreBindings::evaluateFade(
    double offsetSec,
    double durationSec,
    double fadeInSec,
    double fadeOutSec
) noexcept {
    if (durationSec <= 0.0 || offsetSec < 0.0 || offsetSec > durationSec) {
        return 0.0;
    }
    double opacity = 1.0;
    if (fadeInSec > 0.0 && offsetSec < fadeInSec) {
        opacity = std::min(opacity, offsetSec / fadeInSec);
    }
    if (fadeOutSec > 0.0 && offsetSec > (durationSec - fadeOutSec)) {
        opacity = std::min(opacity, (durationSec - offsetSec) / fadeOutSec);
    }
    return std::clamp(opacity, 0.0, 1.0);
}

double OpencutNativeCoreBindings::evaluateMaskAlpha(
    double px, double py,
    int maskType,
    double cx, double cy,
    double sx, double sy,
    double rotDeg,
    double feather,
    bool inverted
) noexcept {
    double rad = -rotDeg * (M_PI / 180.0);
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    double dx = px - cx;
    double dy = py - cy;
    double rx = dx * cosA - dy * sinA;
    double ry = dx * sinA + dy * cosA;

    double halfW = std::max(0.001, sx * 0.5);
    double halfH = std::max(0.001, sy * 0.5);

    double alpha = 0.0;
    if (maskType == 0) { // Rectangle
        double distOuterX = std::abs(rx) - halfW;
        double distOuterY = std::abs(ry) - halfH;
        double maxDist = std::max(distOuterX, distOuterY);

        if (feather <= 0.001) {
            alpha = (maxDist <= 0.0) ? 1.0 : 0.0;
        } else {
            if (maxDist <= -feather) {
                alpha = 1.0;
            } else if (maxDist >= feather) {
                alpha = 0.0;
            } else {
                alpha = 0.5 - (maxDist / (2.0 * feather));
            }
        }
    } else { // Ellipse/Circle
        double nx = rx / halfW;
        double ny = ry / halfH;
        double distSq = nx * nx + ny * ny;
        double dist = std::sqrt(distSq);

        if (feather <= 0.001) {
            alpha = (dist <= 1.0) ? 1.0 : 0.0;
        } else {
            double effectiveRadius = (halfW + halfH) * 0.5;
            double pixelDist = (dist - 1.0) * effectiveRadius;
            if (pixelDist <= -feather) {
                alpha = 1.0;
            } else if (pixelDist >= feather) {
                alpha = 0.0;
            } else {
                alpha = 0.5 - (pixelDist / (2.0 * feather));
            }
        }
    }

    alpha = std::clamp(alpha, 0.0, 1.0);
    if (inverted) {
        alpha = 1.0 - alpha;
    }
    return alpha;
}

double OpencutNativeCoreBindings::solveBezier(double time, double t0, double t1, double t2, double t3) noexcept {
    return core::BezierSolver::solve(time, t0, t1, t2, t3);
}

double OpencutNativeCoreBindings::evaluateBezierPoint(double progress, double p0, double p1, double p2, double p3) noexcept {
    return core::BezierSolver::evaluatePoint(progress, p0, p1, p2, p3);
}

void OpencutNativeCoreBindings::clearBufferRgba(
    uint8_t* buffer,
    int width,
    int height,
    uint8_t r, uint8_t g, uint8_t b, uint8_t a
) noexcept {
    if (!buffer || width <= 0 || height <= 0) return;
    size_t totalPixels = static_cast<size_t>(width) * static_cast<size_t>(height);
    for (size_t i = 0; i < totalPixels; ++i) {
        size_t idx = i * 4;
        buffer[idx + 0] = r;
        buffer[idx + 1] = g;
        buffer[idx + 2] = b;
        buffer[idx + 3] = a;
    }
}

bool OpencutNativeCoreBindings::pointInRotatedRect(
    double px, double py,
    double cx, double cy,
    double w, double h,
    double rotDeg
) noexcept {
    double rad = -rotDeg * (M_PI / 180.0);
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    double dx = px - cx;
    double dy = py - cy;
    double rx = dx * cosA - dy * sinA;
    double ry = dx * sinA + dy * cosA;

    return std::abs(rx) <= (w * 0.5) && std::abs(ry) <= (h * 0.5);
}

void OpencutNativeCoreBindings::applyGainRamp(
    float* samples,
    size_t numSamples,
    float startGain,
    float endGain
) noexcept {
    if (!samples || numSamples == 0) return;
    if (numSamples == 1) {
        samples[0] *= startGain;
        return;
    }
    float step = (endGain - startGain) / static_cast<float>(numSamples - 1);
    for (size_t i = 0; i < numSamples; ++i) {
        float gain = startGain + step * static_cast<float>(i);
        samples[i] *= gain;
    }
}

void OpencutNativeCoreBindings::mixAudioBuffers(
    float* dest,
    const float* src,
    size_t numSamples,
    float volume
) noexcept {
    if (!dest || !src || numSamples == 0) return;
    for (size_t i = 0; i < numSamples; ++i) {
        dest[i] += src[i] * volume;
    }
}

float OpencutNativeCoreBindings::computeBufferPeak(
    const float* samples,
    size_t numSamples
) noexcept {
    if (!samples || numSamples == 0) return 0.0f;
    float peak = 0.0f;
    for (size_t i = 0; i < numSamples; ++i) {
        float val = std::abs(samples[i]);
        if (val > peak) peak = val;
    }
    return peak;
}

void OpencutNativeCoreBindings::clampBufferSamples(
    float* samples,
    size_t numSamples,
    float maxPeak
) noexcept {
    if (!samples || numSamples == 0 || maxPeak <= 0.0f) return;
    for (size_t i = 0; i < numSamples; ++i) {
        samples[i] = std::clamp(samples[i], -maxPeak, maxPeak);
    }
}

void OpencutNativeCoreBindings::downmixStereo(
    const float* left,
    const float* right,
    float* out,
    size_t numSamples
) noexcept {
    if (!left || !right || !out || numSamples == 0) return;
    for (size_t i = 0; i < numSamples; ++i) {
        out[i] = (left[i] + right[i]) * 0.5f;
    }
}

} // namespace catchim::native
