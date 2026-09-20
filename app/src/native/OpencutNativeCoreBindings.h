#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

namespace catchim::native {

class OpencutNativeCoreBindings {
public:
    // Time conversions
    static int64_t ticksPerSecond() noexcept;
    static int64_t fromSeconds(double seconds) noexcept;
    static double toSeconds(int64_t ticks) noexcept;

    // Frame snapping & time rounding
    static int64_t roundToFrame(int64_t ticks, int32_t fpsNum, int32_t fpsDen) noexcept;
    static int64_t floorToFrame(int64_t ticks, int32_t fpsNum, int32_t fpsDen) noexcept;
    static int64_t lastFrame(int64_t durationTicks, int32_t fpsNum, int32_t fpsDen) noexcept;
    static int64_t snappedSeek(int64_t timeTicks, int64_t durationTicks, int32_t fpsNum, int32_t fpsDen) noexcept;

    // Fade and Mask evaluations
    static double evaluateFade(
        double offsetSec,
        double durationSec,
        double fadeInSec,
        double fadeOutSec
    ) noexcept;

    static double evaluateMaskAlpha(
        double px, double py,
        int maskType,
        double cx, double cy,
        double sx, double sy,
        double rotDeg,
        double feather,
        bool inverted
    ) noexcept;

    // Cubic bezier math
    static double solveBezier(double time, double t0, double t1, double t2, double t3) noexcept;
    static double evaluateBezierPoint(double progress, double p0, double p1, double p2, double p3) noexcept;

    // Buffer operations
    static void clearBufferRgba(
        uint8_t* buffer,
        int width,
        int height,
        uint8_t r, uint8_t g, uint8_t b, uint8_t a
    ) noexcept;

    static bool pointInRotatedRect(
        double px, double py,
        double cx, double cy,
        double w, double h,
        double rotDeg
    ) noexcept;

    // Audio operations
    static void applyGainRamp(
        float* samples,
        size_t numSamples,
        float startGain,
        float endGain
    ) noexcept;

    static void mixAudioBuffers(
        float* dest,
        const float* src,
        size_t numSamples,
        float volume
    ) noexcept;

    static float computeBufferPeak(
        const float* samples,
        size_t numSamples
    ) noexcept;

    static void clampBufferSamples(
        float* samples,
        size_t numSamples,
        float maxPeak
    ) noexcept;

    static void downmixStereo(
        const float* left,
        const float* right,
        float* out,
        size_t numSamples
    ) noexcept;
};

} // namespace catchim::native
