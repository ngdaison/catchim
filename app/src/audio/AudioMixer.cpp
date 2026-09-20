#include "AudioMixer.h"
#include <cmath>

namespace catchim::audio {

AudioMixer::AudioMixer(int32_t channels, int32_t sampleRate)
    : channels_(channels), sampleRate_(sampleRate)
{
}

void AudioMixer::mixTimeline(
    const editor::Timeline& timeline,
    core::TimelineTime startTime,
    AudioBuffer& outBuffer
) {
    outBuffer.clear();
    size_t frameCount = outBuffer.frameCount();
    if (frameCount == 0) return;

    double dt = 1.0 / static_cast<double>(sampleRate_);
    core::TimelineTime duration = core::TimelineTime::fromSeconds(frameCount * dt);
    core::TimelineTime endTime = startTime + duration;

    // Mix each unmuted audio track
    for (const auto& track : timeline.audioTracks()) {
        if (track.isMuted()) continue;

        for (const auto& clip : track.clips()) {
            if (clip.isMuted()) continue;

            // Check overlap
            if (clip.startTime() < endTime && clip.endTime() > startTime) {
                double volume = clip.getParam<double>("volume", 1.0);
                float vol = static_cast<float>(std::clamp(volume, 0.0, 2.0));

                // Generate audio samples (or synthesis if no raw file decoded yet)
                for (size_t f = 0; f < frameCount; ++f) {
                    core::TimelineTime sampleTime = startTime + core::TimelineTime::fromSeconds(f * dt);
                    if (sampleTime >= clip.startTime() && sampleTime < clip.endTime()) {
                        // Demo tone or sample
                        float sampleVal = 0.0f; // Silence placeholder if not playing raw PCM
                        for (int32_t ch = 0; ch < channels_; ++ch) {
                            outBuffer.samples()[f * channels_ + ch] += sampleVal * vol;
                        }
                    }
                }
            }
        }
    }

    // Clamp output [-1.0f, 1.0f]
    for (auto& s : outBuffer.samples()) {
        s = std::clamp(s, -1.0f, 1.0f);
    }
}

} // namespace catchim::audio
