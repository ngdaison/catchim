#include "audio/AudioPlaybackEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::audio {

double AudioPlaybackEngine::clampDb(double db) {
    if (!std::isfinite(db)) return 0.0;
    return std::clamp(db, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

double AudioPlaybackEngine::dBToLinear(double db) {
    return std::pow(10.0, clampDb(db) / 20.0);
}

double AudioPlaybackEngine::linearToDb(double linear) {
    if (linear <= 0.00001) return VOLUME_DB_MIN;
    return 20.0 * std::log10(linear);
}

double AudioPlaybackEngine::resolveEffectiveAudioGain(
    const editor::Clip& clip,
    bool trackMuted,
    core::TimelineTime /*localTime*/
) {
    if (trackMuted || clip.isMuted()) {
        return 0.0;
    }

    // Default volume is 0 dB (unity gain = 1.0)
    double volDb = clip.getParam<double>("volume", 0.0);
    return dBToLinear(volDb);
}

std::vector<float> AudioPlaybackEngine::buildWaveformGainSamples(
    const editor::Clip& clip,
    size_t count
) {
    if (count == 0) return {};

    double gain = resolveEffectiveAudioGain(clip, false, core::TimelineTime(0));
    return std::vector<float>(count, static_cast<float>(gain));
}

AudioPlaybackEngine::AudioPlaybackEngine() = default;

AudioBuffer AudioPlaybackEngine::renderAudioSlice(
    const editor::Timeline& timeline,
    core::TimelineTime startTime,
    core::TimelineTime duration,
    int sampleRate
) {
    if (sampleRate <= 0) sampleRate = 48000;
    size_t numFrames = static_cast<size_t>(std::round(duration.toSeconds() * sampleRate));
    if (numFrames == 0) {
        AudioBuffer emptyBuf(2, sampleRate);
        return emptyBuf;
    }

    AudioBuffer masterBuffer(2, sampleRate);
    masterBuffer.resize(numFrames);
    auto& outSamples = masterBuffer.samples();

    core::TimelineTime endTime = startTime + duration;

    auto tracks = timeline.allTracks();
    for (const auto* track : tracks) {
        if (track->isMuted()) continue;

        for (const auto& clip : track->clips()) {
            if (clip.type() != editor::ClipType::Audio && clip.type() != editor::ClipType::Video) {
                continue;
            }

            core::TimelineTime clipStart = clip.startTime();
            core::TimelineTime clipEnd = clipStart + clip.duration();

            if (clipEnd <= startTime || clipStart >= endTime) {
                continue; // No overlap
            }

            // Determine slice overlap window
            core::TimelineTime overlapStart = std::max(startTime, clipStart);
            core::TimelineTime overlapEnd = std::min(endTime, clipEnd);

            size_t frameOffsetInSlice = static_cast<size_t>(
                std::round((overlapStart - startTime).toSeconds() * sampleRate)
            );
            size_t overlapFrames = static_cast<size_t>(
                std::round((overlapEnd - overlapStart).toSeconds() * sampleRate)
            );

            double gain = resolveEffectiveAudioGain(clip, false, overlapStart - clipStart);

            // Synthesize or mix samples into interleaved master buffer
            for (size_t i = 0; i < overlapFrames && (frameOffsetInSlice + i) < numFrames; ++i) {
                size_t frameIdx = frameOffsetInSlice + i;
                double t = (overlapStart.toSeconds() + static_cast<double>(i) / sampleRate);
                // Blended signal proportional to gain
                float sample = static_cast<float>(std::sin(2.0 * 3.1415926535 * 440.0 * t) * 0.2 * gain);

                outSamples[frameIdx * 2 + 0] += sample;
                outSamples[frameIdx * 2 + 1] += sample;
            }
        }
    }

    // Apply master volume
    if (masterVolume_ != 1.0f) {
        for (auto& s : outSamples) {
            s *= masterVolume_;
        }
    }

    // Apply master limiter if enabled
    if (limiterEnabled_ && !outSamples.empty()) {
        AudioMastering::applyLimiter(outSamples.data(), outSamples.size(), static_cast<uint32_t>(sampleRate));
    }

    return masterBuffer;
}

} // namespace catchim::audio
