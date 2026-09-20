#pragma once

#include "editor/project/Project.h"
#include "core/time/TimelineTime.h"
#include "core/time/EuclideanFrameSnapper.h"
#include <functional>
#include <optional>

namespace catchim::editor {

enum class SeekSource {
    Ruler,
    Tracks
};

struct PendingSeekSession {
    SeekSource source;
    double downX{0.0};
    double downY{0.0};
    int64_t downTimeMs{0};
};

struct SeekConfig {
    double zoomLevel{1.0};
    core::TimelineTime duration{0};
    bool isSelecting{false};
    std::optional<core::FrameRate> activeProjectFps{std::nullopt};
    std::function<void()> clearSelectedElements;
    std::function<void(core::TimelineTime)> seek;
    std::function<void(const TimelineViewState&)> setTimelineViewState;
};

class SeekController {
public:
    explicit SeekController(SeekConfig config = {}) : config_(std::move(config)) {}

    void setConfig(SeekConfig config) { config_ = std::move(config); }
    [[nodiscard]] const SeekConfig& config() const noexcept { return config_; }

    static core::TimelineTime pixelToTime(
        double clientX,
        double containerLeft,
        double scrollLeft,
        double zoomLevel,
        core::TimelineTime duration
    ) noexcept;

    static bool isClickGesture(
        double clientX,
        double clientY,
        int64_t timeMs,
        const PendingSeekSession& session
    ) noexcept;

    void onMouseDown(SeekSource source, double clientX, double clientY, int64_t timeMs) noexcept;

    bool onClick(
        SeekSource source,
        double clientX,
        double clientY,
        int64_t timeMs,
        double containerLeft,
        double scrollLeft
    );

    [[nodiscard]] bool isPending() const noexcept { return pendingSession_.has_value(); }
    [[nodiscard]] const std::optional<PendingSeekSession>& session() const noexcept { return pendingSession_; }
    void cancel() noexcept { pendingSession_.reset(); }

private:
    SeekConfig config_;
    std::optional<PendingSeekSession> pendingSession_{std::nullopt};
};

} // namespace catchim::editor
