#include "audio/AudioMastering.h"
#include <algorithm>
#include <cmath>

namespace catchim::audio {

float AudioMastering::computePeak(const float* samples, size_t count) noexcept {
    if (!samples || count == 0) return 0.0f;
    float peak = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float absVal = std::abs(samples[i]);
        if (absVal > peak) {
            peak = absVal;
        }
    }
    return peak;
}

void AudioMastering::downmixStereo(
    const float* left,
    const float* right,
    float* monoOut,
    size_t length
) noexcept {
    if (!left || !right || !monoOut || length == 0) return;
    for (size_t i = 0; i < length; ++i) {
        monoOut[i] = (left[i] + right[i]) * 0.5f;
    }
}

void AudioMastering::applyPanning(
    float* left,
    float* right,
    size_t length,
    float pan
) noexcept {
    if (!left || !right || length == 0) return;
    float p = std::clamp(pan, -1.0f, 1.0f);

    // Constant-power panning law
    float angle = (p + 1.0f) * 0.25f * 3.14159265358979323846f;
    float gainL = std::cos(angle);
    float gainR = std::sin(angle);

    for (size_t i = 0; i < length; ++i) {
        left[i] *= gainL;
        right[i] *= gainR;
    }
}

void AudioMastering::applyLimiter(
    float* samples,
    size_t count,
    uint32_t sampleRate,
    const LimiterConfig& config
) noexcept {
    if (!samples || count == 0 || sampleRate == 0) return;

    double threshLinear = std::pow(10.0, config.thresholdDb / 20.0);
    double attackCoeff = std::exp(-1.0 / (sampleRate * std::max(1e-5, config.attackSec)));
    double releaseCoeff = std::exp(-1.0 / (sampleRate * std::max(1e-5, config.releaseSec)));
    double headroom = std::clamp(config.outputHeadroom, 0.1, 1.0);

    double env = 0.0;
    for (size_t i = 0; i < count; ++i) {
        double rectified = std::abs(static_cast<double>(samples[i]));
        if (rectified > env) {
            env = attackCoeff * env + (1.0 - attackCoeff) * rectified;
        } else {
            env = releaseCoeff * env + (1.0 - releaseCoeff) * rectified;
        }

        double gain = 1.0;
        if (env > threshLinear) {
            double overshootDb = 20.0 * std::log10(env / threshLinear);
            double reductionDb = overshootDb * (1.0 - 1.0 / std::max(1.0, config.ratio));
            gain = std::pow(10.0, -reductionDb / 20.0);
        }

        double out = samples[i] * gain * headroom;
        if (out > headroom) out = headroom;
        else if (out < -headroom) out = -headroom;
        samples[i] = static_cast<float>(out);
    }
}

void AudioMastering::normalizePeak(
    float* samples,
    size_t count,
    float targetPeakDb
) noexcept {
    if (!samples || count == 0) return;
    float currentPeak = computePeak(samples, count);
    if (currentPeak <= 1e-6f) return;

    float targetLinear = std::pow(10.0f, targetPeakDb / 20.0f);
    float factor = targetLinear / currentPeak;

    for (size_t i = 0; i < count; ++i) {
        samples[i] *= factor;
    }
}

} // namespace catchim::audio
