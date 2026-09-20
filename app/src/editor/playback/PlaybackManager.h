#pragma once

#include "core/time/TimelineTime.h"
#include "core/time/RationalFrameRate.h"
#include <vector>
#include <functional>
#include <algorithm>

namespace catchim::editor {

class PlaybackManager {
public:
    using StateListener = std::function<void()>;
    using TimeListener = std::function<void(core::TimelineTime)>;

    PlaybackManager() = default;

    bool isPlaying() const noexcept { return isPlaying_; }
    core::TimelineTime currentTime() const noexcept { return currentTime_; }
    double volume() const noexcept { return volume_; }
    bool isMuted() const noexcept { return isMuted_; }
    bool isScrubbing() const noexcept { return isScrubbing_; }

    void play(core::TimelineTime totalDuration);
    void pause() noexcept;
    void toggle(core::TimelineTime totalDuration);

    void seek(core::TimelineTime time, core::TimelineTime totalDuration);
    void setVolume(double volume) noexcept;
    void mute() noexcept;
    void unmute() noexcept;
    void toggleMute() noexcept;

    void setScrubbing(bool isScrubbing) noexcept;

    void stepForward(core::TimelineTime totalDuration, const core::FrameRate& fps = core::FrameRate{30, 1});
    void stepBackward(core::TimelineTime totalDuration, const core::FrameRate& fps = core::FrameRate{30, 1});

    void reconcileTimelineScope(core::TimelineTime totalDuration);

    void subscribe(StateListener listener);
    void onUpdate(TimeListener listener);
    void onSeek(TimeListener listener);

private:
    void notifyState();
    void notifyTimeUpdate(core::TimelineTime time);
    void notifySeek(core::TimelineTime time);

    bool isPlaying_{false};
    core::TimelineTime currentTime_{0};
    double volume_{1.0};
    bool isMuted_{false};
    double previousVolume_{1.0};
    bool isScrubbing_{false};

    std::vector<StateListener> stateListeners_;
    std::vector<TimeListener> updateListeners_;
    std::vector<TimeListener> seekListeners_;
};

} // namespace catchim::editor
