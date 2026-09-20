#include "PlaybackClock.h"

namespace catchim::editor {

PlaybackClock::PlaybackClock()
    : startTimePoint_(std::chrono::steady_clock::now())
{
}

void PlaybackClock::start() {
    if (isPlaying_) return;
    startTimePoint_ = std::chrono::steady_clock::now();
    isPlaying_ = true;
}

void PlaybackClock::pause() {
    if (!isPlaying_) return;
    baseTime_ = currentTime();
    isPlaying_ = false;
}

void PlaybackClock::seek(core::TimelineTime time) {
    baseTime_ = time;
    startTimePoint_ = std::chrono::steady_clock::now();
}

void PlaybackClock::setPlaybackRate(double rate) {
    if (isPlaying_) {
        baseTime_ = currentTime();
        startTimePoint_ = std::chrono::steady_clock::now();
    }
    rate_ = rate;
}

core::TimelineTime PlaybackClock::currentTime() const {
    if (!isPlaying_) {
        return baseTime_;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsedMicros = std::chrono::duration_cast<std::chrono::microseconds>(now - startTimePoint_).count();
    double elapsedSeconds = (static_cast<double>(elapsedMicros) / 1'000'000.0) * rate_;
    core::TimelineTime elapsedTicks = core::TimelineTime::fromSeconds(elapsedSeconds);

    return baseTime_ + elapsedTicks;
}

} // namespace catchim::editor
