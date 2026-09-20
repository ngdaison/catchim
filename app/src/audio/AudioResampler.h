#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>

namespace catchim::audio {

class AudioResampler {
public:
    // Linear resampling between sample rates (e.g. 48000 Hz -> 44100 Hz)
    static std::vector<float> resampleLinear(
        const float* input,
        size_t inSamples,
        int inSampleRate,
        int outSampleRate
    );

    // Apply fade-in and fade-out volume ramp directly to PCM samples
    static void applyFadeRamp(
        float* buffer,
        size_t totalSamples,
        int sampleRate,
        double fadeInSeconds,
        double fadeOutSeconds,
        double maxVolume = 1.0
    );

    // Calculate instantaneous volume gain at time t within a clip
    static double calculateGainAt(
        double currentTimeSeconds,
        double totalDurationSeconds,
        double fadeInSeconds,
        double fadeOutSeconds,
        double baseVolume = 1.0
    );
};

} // namespace catchim::audio
