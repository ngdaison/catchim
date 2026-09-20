#include "PlaybackManager.h"
#include <cmath>

namespace catchim::editor {

void PlaybackManager::play(core::TimelineTime totalDuration) {
    if (totalDuration.ticks() <= 0) {
        return;
    }

    if (currentTime_ >= totalDuration) {
        seek(core::TimelineTime(0), totalDuration);
    }

    isPlaying_ = true;
    notifyState();
}

void PlaybackManager::pause() noexcept {
    if (isPlaying_) {
        isPlaying_ = false;
        notifyState();
    }
}

void PlaybackManager::toggle(core::TimelineTime totalDuration) {
    if (isPlaying_) {
        pause();
    } else {
        play(totalDuration);
    }
}

void PlaybackManager::seek(core::TimelineTime time, core::TimelineTime totalDuration) {
    core::TimelineTime clamped = time;
    if (clamped.ticks() < 0) {
        clamped = core::TimelineTime(0);
    }
    if (clamped > totalDuration && totalDuration.ticks() > 0) {
        clamped = totalDuration;
    }

    currentTime_ = clamped;
    notifyState();
    notifySeek(currentTime_);
}

void PlaybackManager::setVolume(double volume) noexcept {
    const double clamped = std::clamp(volume, 0.0, 1.0);
    volume_ = clamped;
    isMuted_ = (clamped == 0.0);
    if (clamped > 0.0) {
        previousVolume_ = clamped;
    }
    notifyState();
}

void PlaybackManager::mute() noexcept {
    if (volume_ > 0.0) {
        previousVolume_ = volume_;
    }
    isMuted_ = true;
    volume_ = 0.0;
    notifyState();
}

void PlaybackManager::unmute() noexcept {
    isMuted_ = false;
    volume_ = (previousVolume_ > 0.0) ? previousVolume_ : 1.0;
    notifyState();
}

void PlaybackManager::toggleMute() noexcept {
    if (isMuted_) {
        unmute();
    } else {
        mute();
    }
}

void PlaybackManager::setScrubbing(bool isScrubbing) noexcept {
    if (isScrubbing_ != isScrubbing) {
        isScrubbing_ = isScrubbing;
        notifyState();
    }
}

void PlaybackManager::stepForward(core::TimelineTime totalDuration, const core::FrameRate& fps) {
    const auto frameDur = fps.frameDuration();
    seek(currentTime_ + frameDur, totalDuration);
}

void PlaybackManager::stepBackward(core::TimelineTime totalDuration, const core::FrameRate& fps) {
    const auto frameDur = fps.frameDuration();
    const auto newTicks = std::max<int64_t>(0, currentTime_.ticks() - frameDur.ticks());
    seek(core::TimelineTime(newTicks), totalDuration);
}

void PlaybackManager::reconcileTimelineScope(core::TimelineTime totalDuration) {
    const auto clamped = std::clamp(
        currentTime_,
        core::TimelineTime(0),
        (totalDuration.ticks() > 0) ? totalDuration : core::TimelineTime(0)
    );

    const bool shouldPause = isPlaying_ && (clamped >= totalDuration && totalDuration.ticks() > 0);
    const bool timeChanged = (clamped != currentTime_);

    if (shouldPause) {
        isPlaying_ = false;
    }

    currentTime_ = clamped;
    if (shouldPause || timeChanged) {
        notifyState();
        if (timeChanged) {
            notifySeek(currentTime_);
        }
    }
}

void PlaybackManager::subscribe(StateListener listener) {
    stateListeners_.push_back(std::move(listener));
}

void PlaybackManager::onUpdate(TimeListener listener) {
    updateListeners_.push_back(std::move(listener));
}

void PlaybackManager::onSeek(TimeListener listener) {
    seekListeners_.push_back(std::move(listener));
}

void PlaybackManager::notifyState() {
    for (const auto& listener : stateListeners_) {
        if (listener) listener();
    }
}

void PlaybackManager::notifyTimeUpdate(core::TimelineTime time) {
    for (const auto& listener : updateListeners_) {
        if (listener) listener(time);
    }
}

void PlaybackManager::notifySeek(core::TimelineTime time) {
    for (const auto& listener : seekListeners_) {
        if (listener) listener(time);
    }
}

} // namespace catchim::editor
