#include "AudioManager.h"
#include "AudioStateEngine.h"
#include <algorithm>

namespace catchim::audio {

void AudioManager::setMasterVolume(double vol) noexcept {
    const double clamped = std::clamp(vol, 0.0, 1.0);
    if (masterVolume_ != clamped) {
        masterVolume_ = clamped;
        notify();
    }
}

void AudioManager::setMuted(bool muted) noexcept {
    if (isMuted_ != muted) {
        isMuted_ = muted;
        notify();
    }
}

void AudioManager::setTrackMute(const core::TrackId& trackId, bool muted) {
    if (muted) {
        mutedTracks_.insert(trackId);
    } else {
        mutedTracks_.erase(trackId);
    }
    notify();
}

bool AudioManager::isTrackMuted(const core::TrackId& trackId) const noexcept {
    return mutedTracks_.find(trackId) != mutedTracks_.end();
}

void AudioManager::setTrackSolo(const core::TrackId& trackId, bool solo) {
    if (solo) {
        soloTracks_.insert(trackId);
    } else {
        soloTracks_.erase(trackId);
    }
    notify();
}

bool AudioManager::isTrackSolo(const core::TrackId& trackId) const noexcept {
    return soloTracks_.find(trackId) != soloTracks_.end();
}

std::vector<ActiveAudioClipInfo> AudioManager::collectActiveAudioClips(
    const editor::Timeline& timeline,
    core::TimelineTime currentTime,
    core::TimelineTime lookahead
) const {
    std::vector<ActiveAudioClipInfo> result;
    const auto windowEnd = currentTime + lookahead;
    const bool anySolo = hasSoloTracks();

    for (const auto* track : timeline.allTracks()) {
        if (!track) continue;

        const bool trackMuted = isTrackMuted(track->id()) || track->isMuted();
        const bool trackAllowed = !anySolo || isTrackSolo(track->id());
        const bool effectiveMuted = isMuted_ || trackMuted || !trackAllowed;

        for (const auto& clip : track->clips()) {
            if (clip.type() != editor::ClipType::Audio && clip.type() != editor::ClipType::Video) {
                continue;
            }

            const auto clipStart = clip.startTime();
            const auto clipEnd = clip.endTime();

            // Check if clip overlaps with [currentTime, windowEnd]
            if (clipEnd >= currentTime && clipStart <= windowEnd) {
                const double localTimeSec = std::max(0.0, (currentTime - clipStart).toSeconds());
                const double elemGain = AudioStateEngine::resolveEffectiveAudioGain(
                    clip,
                    effectiveMuted,
                    localTimeSec
                );

                result.push_back(ActiveAudioClipInfo{
                    .clipId = clip.id(),
                    .trackId = track->id(),
                    .clipStartTime = clipStart,
                    .clipDuration = clip.duration(),
                    .effectiveGain = isMuted_ ? 0.0 : (masterVolume_ * elemGain),
                    .isMuted = effectiveMuted || AudioStateEngine::isElementMuted(clip)
                });
            }
        }
    }

    return result;
}

void AudioManager::subscribe(AudioChangeListener listener) {
    listeners_.push_back(std::move(listener));
}

void AudioManager::notify() {
    for (const auto& l : listeners_) {
        if (l) l();
    }
}

} // namespace catchim::audio
