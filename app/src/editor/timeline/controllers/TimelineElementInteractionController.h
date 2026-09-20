#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <functional>
#include <optional>
#include <vector>

namespace catchim::editor {

struct MousedownSnapshot {
    double originX{0.0};
    double originY{0.0};
    core::ClipId elementId;
    core::TrackId trackId;
    core::TimelineTime startElementTime{0};
    core::TimelineTime clickOffsetTime{0};
    std::vector<core::ClipId> selectedElementIds;
};

struct ElementInteractionConfig {
    double zoomLevel{1.0};
    bool isShiftHeld{false};
    std::function<void(const std::vector<core::ClipId>&, core::TimelineTime)> onCommitMoves;
    std::function<void(const core::ClipId&, bool isMultiKey)> onElementClick;
    std::function<void(const std::vector<core::ClipId>&, core::TimelineTime)> onPreviewMoves;
};

class TimelineElementInteractionController {
public:
    explicit TimelineElementInteractionController(ElementInteractionConfig config = {})
        : config_(std::move(config)) {}

    void setConfig(ElementInteractionConfig config) { config_ = std::move(config); }
    [[nodiscard]] const ElementInteractionConfig& config() const noexcept { return config_; }

    [[nodiscard]] bool isDragging() const noexcept { return isDragging_; }
    [[nodiscard]] bool hasSession() const noexcept { return snapshot_.has_value(); }
    [[nodiscard]] core::TimelineTime currentDeltaTime() const noexcept { return currentDeltaTime_; }

    void onMouseDown(
        core::TrackId trackId,
        core::ClipId elementId,
        core::TimelineTime startTime,
        double clientX,
        double clientY,
        const std::vector<core::ClipId>& selectedIds
    );

    void handleMouseMove(double clientX, double clientY);
    void handleMouseUp(double clientX, double clientY, bool isMultiKey);
    void cancel() noexcept;

private:
    ElementInteractionConfig config_;
    std::optional<MousedownSnapshot> snapshot_{std::nullopt};
    bool isDragging_{false};
    core::TimelineTime currentDeltaTime_{0};
};

} // namespace catchim::editor
