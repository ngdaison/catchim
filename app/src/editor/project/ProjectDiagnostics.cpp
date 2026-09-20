#include "editor/project/ProjectDiagnostics.h"
#include <algorithm>

namespace catchim::editor {

std::vector<TimelineGap> ProjectDiagnostics::findGapsOnTrack(const Track& track) {
    std::vector<TimelineGap> gaps;
    const auto& clips = track.clips();
    if (clips.size() < 2) return gaps;

    // Track clips are always sorted by startTime
    for (size_t i = 0; i < clips.size() - 1; ++i) {
        core::TimelineTime endA = clips[i].endTime();
        core::TimelineTime startB = clips[i + 1].startTime();

        if (endA < startB) {
            TimelineGap gap;
            gap.trackId = track.id();
            gap.startTime = endA;
            gap.duration = startB - endA;
            gaps.push_back(gap);
        }
    }

    return gaps;
}

DiagnosticReport ProjectDiagnostics::analyze(const Project& project) {
    DiagnosticReport report;
    report.totalScenes = project.scenes().size();
    report.totalDuration = project.totalDuration();

    for (const auto& scene : project.scenes()) {
        for (const auto* track : scene.timeline().allTracks()) {
            report.totalTracks++;
            report.totalClips += track->clips().size();

            // Find gaps
            auto trackGaps = findGapsOnTrack(*track);
            report.gaps.insert(report.gaps.end(), trackGaps.begin(), trackGaps.end());

            // Check for overlaps
            const auto& clips = track->clips();
            for (size_t i = 0; i + 1 < clips.size(); ++i) {
                if (clips[i].endTime() > clips[i + 1].startTime()) {
                    report.overlapCount++;
                }
            }
        }
    }

    return report;
}

} // namespace catchim::editor
