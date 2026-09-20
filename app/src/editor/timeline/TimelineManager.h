#pragma once

#include "editor/project/Project.h"
#include "editor/timeline/Timeline.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "core/time/RationalFrameRate.h"
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace catchim::editor {

class TimelineManager {
public:
    using TimelineChangeListener = std::function<void()>;

    explicit TimelineManager(Project& project);

    Project& project() noexcept { return project_; }
    const Project& project() const noexcept { return project_; }

    core::TimelineTime getTotalDuration() const noexcept;
    core::TimelineTime getLastFrameTime(const core::FrameRate& fps = core::FrameRate{30, 1}) const noexcept;

    // Track operations
    core::TrackId addTrack(TrackType type, std::string name);
    bool removeTrack(const core::TrackId& trackId);
    bool toggleTrackMute(const core::TrackId& trackId);
    bool toggleTrackVisibility(const core::TrackId& trackId);

    // Element operations
    bool insertElement(const core::TrackId& trackId, Clip clip);
    bool deleteElements(const std::vector<core::ClipId>& clipIds);
    std::vector<core::ClipId> duplicateElements(const std::vector<core::ClipId>& clipIds);
    bool splitElements(const std::vector<core::ClipId>& clipIds, core::TimelineTime splitTime);
    bool moveElement(const core::ClipId& clipId, const core::TrackId& newTrackId, core::TimelineTime newStartTime);

    // Observers
    void subscribe(TimelineChangeListener listener);
    void notify();

private:
    Timeline* activeTimeline() noexcept;
    const Timeline* activeTimeline() const noexcept;

    Project& project_;
    std::vector<TimelineChangeListener> listeners_;
};

} // namespace catchim::editor
