#pragma once

#include "PlaybackClock.h"
#include "core/time/TimelineTime.h"
#include <functional>

namespace catchim::editor {

class PlaybackController {
public:
    PlaybackController();

    void play();
    void pause();
    void toggle();
    void stop();

    void seek(core::TimelineTime time);
    void stepFrame(int frames, const core::FrameRate& rate);
    void jumpSeconds(double seconds);

    bool isPlaying() const noexcept { return clock_.isPlaying(); }
    core::TimelineTime currentTime() const;
    void setPlaybackRate(double rate) { clock_.setPlaybackRate(rate); }

    void setDuration(core::TimelineTime duration) { duration_ = duration; }
    core::TimelineTime duration() const noexcept { return duration_; }

    // Callbacks
    void setOnTimeChanged(std::function<void(core::TimelineTime)> cb) { onTimeChanged_ = std::move(cb); }
    void setOnPlayStateChanged(std::function<void(bool)> cb) { onPlayStateChanged_ = std::move(cb); }

    // Update tick from app loop
    void update();

private:
    PlaybackClock clock_;
    core::TimelineTime duration_{core::TimelineTime(0)};
    std::function<void(core::TimelineTime)> onTimeChanged_;
    std::function<void(bool)> onPlayStateChanged_;
};

} // namespace catchim::editor
