#include "audio/AudioDucker.h"
#include <cmath>
#include <algorithm>

namespace catchim::audio {

void AudioDucker::applyDucking(
    AudioBuffer& musicBuffer,
    const AudioBuffer& voiceBuffer,
    const DuckingConfig& config
) noexcept {
    size_t numFrames = std::min(musicBuffer.frameCount(), voiceBuffer.frameCount());
    if (numFrames == 0) return;

    int channels = musicBuffer.channels();
    int voiceChannels = voiceBuffer.channels();
    int sampleRate = musicBuffer.sampleRate();

    float thresholdLinear = static_cast<float>(std::pow(10.0, config.thresholdDb / 20.0));
    float duckingLinear = static_cast<float>(std::pow(10.0, config.duckingDb / 20.0));

    double attackSec = std::max(0.001, config.attackTimeSec);
    double releaseSec = std::max(0.001, config.releaseTimeSec);

    float attackCoeff = static_cast<float>(std::exp(-1.0 / (attackSec * sampleRate)));
    float releaseCoeff = static_cast<float>(std::exp(-1.0 / (releaseSec * sampleRate)));
    size_t holdFrames = static_cast<size_t>(config.holdTimeSec * sampleRate);

    auto& musicSamples = musicBuffer.samples();
    const auto& voiceSamples = voiceBuffer.samples();

    float currentGain = 1.0f;
    size_t holdCounter = 0;

    for (size_t f = 0; f < numFrames; ++f) {
        // Detect voice energy in this frame
        float voiceLevel = 0.0f;
        for (int ch = 0; ch < voiceChannels; ++ch) {
            voiceLevel = std::max(voiceLevel, std::abs(voiceSamples[f * voiceChannels + ch]));
        }

        float targetGain = 1.0f;
        if (voiceLevel >= thresholdLinear) {
            targetGain = duckingLinear;
            holdCounter = holdFrames;
        } else if (holdCounter > 0) {
            --holdCounter;
            targetGain = duckingLinear;
        } else {
            targetGain = 1.0f;
        }

        // Apply smooth envelope filter
        if (targetGain < currentGain) {
            currentGain = targetGain + attackCoeff * (currentGain - targetGain);
        } else {
            currentGain = targetGain + releaseCoeff * (currentGain - targetGain);
        }

        // Attenuate music
        for (int ch = 0; ch < channels; ++ch) {
            musicSamples[f * channels + ch] *= currentGain;
        }
    }
}

} // namespace catchim::audio
