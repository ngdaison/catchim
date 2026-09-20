#pragma once

#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include "core/time/TimelineTime.h"
#include "core/ids/Ids.h"
#include <vector>
#include <unordered_set>
#include <functional>

namespace catchim::audio {

struct ActiveAudioClipInfo {
    core::ClipId clipId;
    core::TrackId trackId;
    core::TimelineTime clipStartTime{0};
    core::TimelineTime clipDuration{0};
    double effectiveGain{1.0};
    bool isMuted{false};
};

class AudioManager {
public:
    using AudioChangeListener = std::function<void()>;

    AudioManager() = default;

    double masterVolume() const noexcept { return masterVolume_; }
    void setMasterVolume(double vol) noexcept;

    bool isMuted() const noexcept { return isMuted_; }
    void setMuted(bool muted) noexcept;

    void setTrackMute(const core::TrackId& trackId, bool muted);
    bool isTrackMuted(const core::TrackId& trackId) const noexcept;

    void setTrackSolo(const core::TrackId& trackId, bool solo);
    bool isTrackSolo(const core::TrackId& trackId) const noexcept;
    bool hasSoloTracks() const noexcept { return !soloTracks_.empty(); }

    std::vector<ActiveAudioClipInfo> collectActiveAudioClips(
        const editor::Timeline& timeline,
        core::TimelineTime currentTime,
        core::TimelineTime lookahead = core::TimelineTime::fromSeconds(2.0)
    ) const;

    void subscribe(AudioChangeListener listener);

private:
    void notify();

    double masterVolume_{1.0};
    bool isMuted_{false};
    std::unordered_set<core::TrackId> mutedTracks_;
    std::unordered_set<core::TrackId> soloTracks_;
    std::vector<AudioChangeListener> listeners_;
};

} // namespace catchim::audio
