#pragma once

#include "editor/timeline/GroupResizeEngine.h"
#include "core/time/TimelineTime.h"
#include <functional>
#include <optional>
#include <vector>

namespace catchim::editor {

struct ResizeConfig {
    double zoomLevel{1.0};
    bool snappingEnabled{true};
    bool isShiftHeld{false};
    core::FrameRate fps{30, 1};
    std::function<void(const std::vector<GroupResizeUpdate>&)> previewElements;
    std::function<void(const std::vector<GroupResizeUpdate>&)> commitElements;
    std::function<void()> discardPreview;
    std::function<std::vector<core::TimelineTime>()> snapPointsProvider;
    std::function<void(std::optional<core::TimelineTime>)> onSnapPointChange;
};

struct ActiveResizeSession {
    ResizeSide side{ResizeSide::Right};
    double startX{0.0};
    core::FrameRate fps{30, 1};
    std::vector<GroupResizeMember> members;
    std::optional<GroupResizeResult> result{std::nullopt};
};

class ResizeController {
public:
    explicit ResizeController(ResizeConfig config = {}) : config_(std::move(config)) {}

    void setConfig(ResizeConfig config) { config_ = std::move(config); }
    [[nodiscard]] const ResizeConfig& config() const noexcept { return config_; }

    [[nodiscard]] bool isResizing() const noexcept { return session_.has_value(); }
    [[nodiscard]] const std::optional<ActiveResizeSession>& session() const noexcept { return session_; }

    void onResizeStart(
        ResizeSide side,
        double clientX,
        std::vector<GroupResizeMember> members
    );

    void handleMouseMove(double clientX);
    void handleMouseUp();
    void cancel();

    static bool hasResizeChanges(
        const std::vector<GroupResizeMember>& members,
        const GroupResizeResult& result
    ) noexcept;

    core::TimelineTime computeSnappedDelta(
        const ActiveResizeSession& session,
        core::TimelineTime rawDeltaTime
    ) const;

private:
    ResizeConfig config_;
    std::optional<ActiveResizeSession> session_{std::nullopt};
};

} // namespace catchim::editor
