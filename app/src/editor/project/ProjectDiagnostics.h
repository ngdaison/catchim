#pragma once

#include "editor/project/Project.h"
#include <vector>

namespace catchim::editor {

struct TimelineGap {
    core::TrackId trackId;
    core::TimelineTime startTime;
    core::TimelineTime duration;

    [[nodiscard]] core::TimelineTime endTime() const noexcept {
        return startTime + duration;
    }
};

struct DiagnosticReport {
    size_t totalScenes{0};
    size_t totalTracks{0};
    size_t totalClips{0};
    core::TimelineTime totalDuration{0};
    std::vector<TimelineGap> gaps;
    size_t overlapCount{0};

    [[nodiscard]] size_t gapCount() const noexcept { return gaps.size(); }
    [[nodiscard]] bool isClean() const noexcept { return gaps.empty() && overlapCount == 0; }
};

class ProjectDiagnostics {
public:
    static DiagnosticReport analyze(const Project& project);
    static std::vector<TimelineGap> findGapsOnTrack(const Track& track);
};

} // namespace catchim::editor
