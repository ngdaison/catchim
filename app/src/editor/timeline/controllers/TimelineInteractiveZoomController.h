#pragma once

#include "core/time/TimelineTime.h"
#include "editor/project/Project.h"
#include <functional>
#include <optional>
#include <algorithm>

namespace catchim::editor {

struct InteractiveZoomConfig {
    double minZoom{0.1};
    double maxZoom{100.0};
    std::function<core::TimelineTime()> getCurrentPlayheadTime;
    std::function<void(const TimelineViewState&)> onViewStateChanged;
};

class TimelineInteractiveZoomController {
public:
    static constexpr double TIMELINE_ZOOM_ANCHOR_PLAYHEAD_THRESHOLD = 0.25;

    explicit TimelineInteractiveZoomController(InteractiveZoomConfig config, double initialZoom = 1.0);

    double zoomLevel() const noexcept { return zoomLevel_; }
    double scrollLeft() const noexcept { return scrollLeft_; }
    bool isInPlayheadAnchorMode() const noexcept { return isInPlayheadAnchorMode_; }

    static double clampZoom(double zoomLevel, double minZoom, double maxZoom = 100.0) noexcept;

    void setZoomLevel(double newZoom, double scrollWidth = 0.0, double clientWidth = 0.0);
    void updateZoom(std::function<double(double)> updater, double scrollWidth = 0.0, double clientWidth = 0.0);

    bool handleWheel(
        double deltaX,
        double deltaY,
        bool isCtrlOrMeta,
        bool isShift,
        double scrollWidth = 0.0,
        double clientWidth = 0.0
    );

    void setScrollLeft(double scrollLeft) noexcept { scrollLeft_ = scrollLeft; }
    void reconcileInitialAndMinZoom(double minZoom, std::optional<double> initialZoom = std::nullopt);

private:
    void applyZoomLayout(double newZoom, double scrollWidth, double clientWidth);

    InteractiveZoomConfig config_;
    double zoomLevel_{1.0};
    double previousZoom_{1.0};
    double scrollLeft_{0.0};
    double preZoomScrollLeft_{0.0};
    double prePlayheadAnchorScrollLeft_{0.0};
    bool isInPlayheadAnchorMode_{false};
    bool hasInitialized_{false};
};

} // namespace catchim::editor
