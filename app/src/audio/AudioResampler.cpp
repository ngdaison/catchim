#include "audio/AudioResampler.h"
#include <algorithm>
#include <cmath>

namespace catchim::audio {

std::vector<float> AudioResampler::resampleLinear(
    const float* input,
    size_t inSamples,
    int inSampleRate,
    int outSampleRate
) {
    if (!input || inSamples == 0 || inSampleRate <= 0 || outSampleRate <= 0) {
        return {};
    }

    if (inSampleRate == outSampleRate) {
        return std::vector<float>(input, input + inSamples);
    }

    double ratio = static_cast<double>(inSampleRate) / static_cast<double>(outSampleRate);
    size_t outSamples = static_cast<size_t>(std::round(inSamples / ratio));
    if (outSamples == 0) return {};

    std::vector<float> output(outSamples);

    for (size_t i = 0; i < outSamples; ++i) {
        double srcPos = i * ratio;
        size_t idx0 = static_cast<size_t>(srcPos);
        size_t idx1 = std::min(idx0 + 1, inSamples - 1);
        double frac = srcPos - idx0;

        if (idx0 < inSamples) {
            output[i] = static_cast<float>((1.0 - frac) * input[idx0] + frac * input[idx1]);
        } else {
            output[i] = input[inSamples - 1];
        }
    }

    return output;
}

double AudioResampler::calculateGainAt(
    double currentTimeSeconds,
    double totalDurationSeconds,
    double fadeInSeconds,
    double fadeOutSeconds,
    double baseVolume
) {
    if (currentTimeSeconds < 0.0 || currentTimeSeconds > totalDurationSeconds) {
        return 0.0;
    }

    double gain = baseVolume;

    // Fade In
    if (fadeInSeconds > 0.001) {
        double inRatio = currentTimeSeconds / fadeInSeconds;
        gain *= std::clamp(inRatio, 0.0, 1.0);
    }

    // Fade Out
    if (fadeOutSeconds > 0.001) {
        double timeFromEnd = totalDurationSeconds - currentTimeSeconds;
        double outRatio = timeFromEnd / fadeOutSeconds;
        gain *= std::clamp(outRatio, 0.0, 1.0);
    }

    return gain;
}

void AudioResampler::applyFadeRamp(
    float* buffer,
    size_t totalSamples,
    int sampleRate,
    double fadeInSeconds,
    double fadeOutSeconds,
    double maxVolume
) {
    if (!buffer || totalSamples == 0 || sampleRate <= 0) return;

    double totalDuration = static_cast<double>(totalSamples) / sampleRate;

    for (size_t i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / sampleRate;
        double gain = calculateGainAt(t, totalDuration, fadeInSeconds, fadeOutSeconds, maxVolume);
        buffer[i] = static_cast<float>(buffer[i] * gain);
    }
}

} // namespace catchim::audio
