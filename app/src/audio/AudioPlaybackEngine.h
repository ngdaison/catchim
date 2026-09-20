#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/Clip.h"
#include "editor/timeline/Timeline.h"
#include "audio/AudioBuffer.h"
#include "audio/AudioMastering.h"
#include <vector>
#include <memory>

namespace catchim::media {
class MediaLibrary;
}

namespace catchim::audio {

class AudioPlaybackEngine {
public:
    static constexpr double VOLUME_DB_MIN = -60.0;
    static constexpr double VOLUME_DB_MAX = 12.0;

    static double clampDb(double db);
    static double dBToLinear(double db);
    static double linearToDb(double linear);

    static double resolveEffectiveAudioGain(
        const editor::Clip& clip,
        bool trackMuted,
        core::TimelineTime localTime
    );

    static std::vector<float> buildWaveformGainSamples(
        const editor::Clip& clip,
        size_t count
    );

    AudioPlaybackEngine();

    void setMasterVolume(float volume) { masterVolume_ = std::clamp(volume, 0.0f, 2.0f); }
    float getMasterVolume() const { return masterVolume_; }

    void setMasterLimiterEnabled(bool enabled) { limiterEnabled_ = enabled; }
    bool isMasterLimiterEnabled() const { return limiterEnabled_; }

    AudioBuffer renderAudioSlice(
        const editor::Timeline& timeline,
        core::TimelineTime startTime,
        core::TimelineTime duration,
        int sampleRate = 48000,
        const media::MediaLibrary* mediaLibrary = nullptr
    );

private:
    float masterVolume_{1.0f};
    bool limiterEnabled_{true};
};

} // namespace catchim::audio
