#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>

namespace catchim::audio {

struct LimiterConfig {
    double thresholdDb{-1.0};
    double kneeDb{0.0};
    double ratio{20.0};
    double attackSec{0.001};
    double releaseSec{0.12};
    double outputHeadroom{0.98};
};

class AudioMastering {
public:
    static float computePeak(const float* samples, size_t count) noexcept;

    static void downmixStereo(
        const float* left,
        const float* right,
        float* monoOut,
        size_t length
    ) noexcept;

    static void applyPanning(
        float* left,
        float* right,
        size_t length,
        float pan
    ) noexcept;

    static void applyLimiter(
        float* samples,
        size_t count,
        uint32_t sampleRate,
        const LimiterConfig& config = {}
    ) noexcept;

    static void normalizePeak(
        float* samples,
        size_t count,
        float targetPeakDb = -0.5f
    ) noexcept;
};

} // namespace catchim::audio
