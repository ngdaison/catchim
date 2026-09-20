#pragma once

#include "core/time/TimelineTime.h"
#include <chrono>

namespace catchim::editor {

class PlaybackClock {
public:
    PlaybackClock();

    void start();
    void pause();
    void seek(core::TimelineTime time);

    bool isPlaying() const noexcept { return isPlaying_; }

    core::TimelineTime currentTime() const;
    void setPlaybackRate(double rate);
    double playbackRate() const noexcept { return rate_; }

private:
    bool isPlaying_{false};
    double rate_{1.0};
    core::TimelineTime baseTime_{core::TimelineTime(0)};
    mutable std::chrono::steady_clock::time_point startTimePoint_;
};

} // namespace catchim::editor
