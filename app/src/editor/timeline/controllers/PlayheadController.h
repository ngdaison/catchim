#pragma once

#include "SeekController.h"
#include "core/time/TimelineTime.h"
#include "core/time/EuclideanFrameSnapper.h"
#include <functional>
#include <optional>
#include <vector>

namespace catchim::editor {

struct PlayheadScrubSession {
    bool didStartFromRuler{false};
    bool hasMoved{false};
    std::optional<core::TimelineTime> currentTime{std::nullopt};
};

struct PlayheadConfig {
    double zoomLevel{1.0};
    core::TimelineTime duration{0};
    std::optional<core::FrameRate> activeProjectFps{std::nullopt};
    bool isShiftHeld{false};
    bool isPlaying{false};
    std::function<void(core::TimelineTime)> seek;
    std::function<void(bool)> setScrubbing;
    std::function<void(const TimelineViewState&)> setTimelineViewState;
    std::function<std::vector<core::TimelineTime>()> snapPointsProvider;
};

class PlayheadController {
public:
    explicit PlayheadController(PlayheadConfig config = {}) : config_(std::move(config)) {}

    void setConfig(PlayheadConfig config) { config_ = std::move(config); }
    [[nodiscard]] const PlayheadConfig& config() const noexcept { return config_; }

    static core::TimelineTime pixelToTime(
        double clientX,
        double rulerLeft,
        double zoomLevel,
        core::TimelineTime duration
    ) noexcept;

    void onPlayheadMouseDown(double clientX, double rulerLeft);
    void onRulerMouseDown(double clientX, double rulerLeft);

    void handleMouseMove(double clientX, double rulerLeft);
    void handleMouseUp(double clientX, double rulerLeft, double scrollLeft);

    /**
     * @brief Auto-scrolls viewport to keep playhead visible during playback.
     * Returns true if scroll position changed.
     */
    bool handlePlaybackUpdate(
        core::TimelineTime time,
        double viewportWidth,
        double contentWidth,
        double& inOutScrollLeft
    );

    [[nodiscard]] bool isScrubbing() const noexcept { return session_.has_value(); }
    [[nodiscard]] const std::optional<PlayheadScrubSession>& session() const noexcept { return session_; }
    [[nodiscard]] double lastMouseClientX() const noexcept { return lastMouseClientX_; }

private:
    void scrub(double clientX, double rulerLeft, bool isElementSnappingEnabled);

    PlayheadConfig config_;
    std::optional<PlayheadScrubSession> session_{std::nullopt};
    double lastMouseClientX_{0.0};
};

} // namespace catchim::editor
