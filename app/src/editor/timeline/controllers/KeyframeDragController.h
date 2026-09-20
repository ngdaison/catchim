#pragma once

#include "core/time/TimelineTime.h"
#include "core/time/EuclideanFrameSnapper.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace catchim::editor {

enum class KeyframeDragSessionKind {
    Idle,
    Pending,
    Active
};

struct KeyframeDragSession {
    KeyframeDragSessionKind kind{KeyframeDragSessionKind::Idle};
    std::vector<std::string> keyframeIds;
    double startMouseX{0.0};
    int64_t deltaTicks{0};
};

struct KeyframeDragState {
    bool isDragging{false};
    std::vector<std::string> draggingKeyframeIds;
    int64_t deltaTicks{0};
};

struct KeyframeDragConfig {
    double zoomLevel{1.0};
    std::optional<core::FrameRate> fps{std::nullopt};
    core::TimelineTime elementDuration{0};
    core::TimelineTime displayedStartTime{0};
    std::function<void(const std::vector<std::string>&, int64_t)> commitDrag;
    std::function<void(core::TimelineTime)> seek;
    std::function<core::TimelineTime()> getTotalDuration;
};

class KeyframeDragController {
public:
    explicit KeyframeDragController(KeyframeDragConfig config = {}) : config_(std::move(config)) {}

    void setConfig(KeyframeDragConfig config) { config_ = std::move(config); }
    [[nodiscard]] const KeyframeDragConfig& config() const noexcept { return config_; }

    [[nodiscard]] bool isActive() const noexcept {
        return session_.kind != KeyframeDragSessionKind::Idle;
    }

    [[nodiscard]] KeyframeDragState dragState() const noexcept;

    void onKeyframeMouseDown(const std::vector<std::string>& keyframes, double clientX) noexcept;
    bool onKeyframeClick(
        const std::vector<std::string>& keyframes,
        double clientX,
        core::TimelineTime indicatorTime
    );

    void handleMouseMove(double clientX) noexcept;
    void handleMouseUp();

    void cancel() noexcept;

    static core::TimelineTime calculateClampedTime(
        core::TimelineTime originalTime,
        int64_t deltaTicks,
        core::TimelineTime maxDuration
    ) noexcept;

    [[nodiscard]] double getVisualOffsetPx(
        core::TimelineTime indicatorTime,
        double indicatorOffsetPx,
        bool isBeingDragged,
        double elementLeft
    ) const noexcept;

private:
    KeyframeDragConfig config_;
    KeyframeDragSession session_;
    std::optional<double> mouseDownX_{std::nullopt};
};

} // namespace catchim::editor
