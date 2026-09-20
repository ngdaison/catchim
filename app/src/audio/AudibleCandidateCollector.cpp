#include "AudibleCandidateCollector.h"
#include <algorithm>

namespace catchim::audio {

std::vector<AudibleCandidate> AudibleCandidateCollector::collectAudibleCandidates(
    const editor::Timeline& timeline,
    const std::unordered_map<std::string, bool>& mediaHasAudioMap
) {
    std::vector<AudibleCandidate> candidates;

    for (const auto* track : timeline.allTracks()) {
        if (!track || track->isMuted()) {
            continue;
        }

        for (const auto& clip : track->clips()) {
            if (clip.duration() <= core::TimelineTime::zero()) {
                continue;
            }
            if (clip.isHidden()) {
                continue;
            }

            bool hasAudio = false;
            if (clip.type() == editor::ClipType::Audio) {
                hasAudio = true;
            } else if (clip.type() == editor::ClipType::Video) {
                if (!clip.mediaId().isEmpty()) {
                    auto it = mediaHasAudioMap.find(clip.mediaId().str());
                    if (it != mediaHasAudioMap.end()) {
                        hasAudio = it->second;
                    }
                }
            }

            if (!hasAudio) {
                continue;
            }

            double vol = 1.0;
            if (clip.params().contains("volume") && clip.params()["volume"].is_number()) {
                vol = clip.params()["volume"].get<double>();
            }

            candidates.push_back(AudibleCandidate{
                .trackId = track->id(),
                .clipId = clip.id(),
                .type = clip.type(),
                .mediaId = clip.mediaId(),
                .startTime = clip.startTime(),
                .duration = clip.duration(),
                .trimStart = clip.trimStart(),
                .trimEnd = clip.trimEnd(),
                .muted = clip.isMuted(),
                .volume = vol
            });
        }
    }

    return candidates;
}

bool AudibleCandidateCollector::timelineHasAudio(
    const editor::Timeline& timeline,
    const std::unordered_map<std::string, bool>& mediaHasAudioMap
) noexcept {
    auto candidates = collectAudibleCandidates(timeline, mediaHasAudioMap);
    for (const auto& c : candidates) {
        if (!c.muted) {
            return true;
        }
    }
    return false;
}

core::TimelineTime AudibleCandidateCollector::calculateTotalAudibleDuration(
    const std::vector<AudibleCandidate>& candidates
) noexcept {
    core::TimelineTime maxEnd = core::TimelineTime::zero();
    for (const auto& c : candidates) {
        if (!c.muted) {
            core::TimelineTime end = c.startTime + c.duration;
            if (end > maxEnd) {
                maxEnd = end;
            }
        }
    }
    return maxEnd;
}

core::TimelineTime AudibleCandidateCollector::calculateTotalAudibleDuration(
    const editor::Timeline& timeline,
    const std::unordered_map<std::string, bool>& mediaHasAudioMap
) noexcept {
    return calculateTotalAudibleDuration(collectAudibleCandidates(timeline, mediaHasAudioMap));
}

} // namespace catchim::audio
