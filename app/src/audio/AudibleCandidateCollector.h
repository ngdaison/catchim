#pragma once

#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <unordered_map>
#include <string>

namespace catchim::audio {

struct AudibleCandidate {
    core::TrackId trackId{core::TrackId::empty()};
    core::ClipId clipId{core::ClipId::empty()};
    editor::ClipType type{editor::ClipType::Audio};
    core::MediaId mediaId{core::MediaId::empty()};
    core::TimelineTime startTime{core::TimelineTime::zero()};
    core::TimelineTime duration{core::TimelineTime::zero()};
    core::TimelineTime trimStart{core::TimelineTime::zero()};
    core::TimelineTime trimEnd{core::TimelineTime::zero()};
    bool muted{false};
    double volume{1.0};

    bool operator==(const AudibleCandidate& other) const = default;
};

class AudibleCandidateCollector {
public:
    static std::vector<AudibleCandidate> collectAudibleCandidates(
        const editor::Timeline& timeline,
        const std::unordered_map<std::string, bool>& mediaHasAudioMap = {}
    );

    static bool timelineHasAudio(
        const editor::Timeline& timeline,
        const std::unordered_map<std::string, bool>& mediaHasAudioMap = {}
    ) noexcept;

    static core::TimelineTime calculateTotalAudibleDuration(
        const std::vector<AudibleCandidate>& candidates
    ) noexcept;

    static core::TimelineTime calculateTotalAudibleDuration(
        const editor::Timeline& timeline,
        const std::unordered_map<std::string, bool>& mediaHasAudioMap = {}
    ) noexcept;
};

} // namespace catchim::audio
