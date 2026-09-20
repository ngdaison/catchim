#pragma once

#include "AudioBuffer.h"
#include "editor/timeline/Timeline.h"
#include "core/time/TimelineTime.h"

namespace catchim::audio {

class AudioMixer {
public:
    AudioMixer(int32_t channels = 2, int32_t sampleRate = 44100);

    void mixTimeline(
        const editor::Timeline& timeline,
        core::TimelineTime startTime,
        AudioBuffer& outBuffer
    );

    int32_t sampleRate() const noexcept { return sampleRate_; }
    int32_t channels() const noexcept { return channels_; }

private:
    int32_t channels_{2};
    int32_t sampleRate_{44100};
};

} // namespace catchim::audio
