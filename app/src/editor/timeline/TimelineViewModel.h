#pragma once

#include "core/time/TimelineTime.h"
#include "editor/EditorEngine.h"
#include "editor/timeline/RulerEngine.h"
#include "editor/timeline/TimelineZoomController.h"
#include <vector>
#include <optional>

namespace catchim::editor {

struct TimeRange {
    core::TimelineTime start;
    core::TimelineTime end;
};

struct MarqueeRect {
    double startX{0.0};
    double startY{0.0};
    double currentX{0.0};
    double currentY{0.0};

    double minX() const { return std::min(startX, currentX); }
    double maxX() const { return std::max(startX, currentX); }
    double minY() const { return std::min(startY, currentY); }
    double maxY() const { return std::max(startY, currentY); }
    double width() const { return std::abs(currentX - startX); }
    double height() const { return std::abs(currentY - startY); }
};

struct TrackVisualLayout {
    int trackIndex{0};
    core::TrackId trackId;
    double topY{0.0};
    double height{56.0};
    bool isCollapsed{false};
    bool isMuted{false};
    bool isLocked{false};
};

class TimelineViewModel {
public:
    explicit TimelineViewModel(EditorEngine& engine);

    EditorEngine& getEngine() { return engine_; }
    const EditorEngine& getEngine() const { return engine_; }

    TimelineZoomController& getZoomController() { return zoomController_; }
    const TimelineZoomController& getZoomController() const { return zoomController_; }

    // Viewport geometry & scroll
    void setViewportWidth(double widthPixels);
    double getViewportWidth() const { return viewportWidth_; }

    void setScrollOffsetX(double scrollX);
    double getScrollOffsetX() const { return scrollOffsetX_; }

    void setScrollOffsetY(double scrollY);
    double getScrollOffsetY() const { return scrollOffsetY_; }

    TimeRange getVisibleTimeRange() const;

    // Coordinate conversions
    double timeToPixel(core::TimelineTime time) const;
    core::TimelineTime pixelToTime(double pixelX) const;

    // Playhead auto-scroll
    bool ensurePlayheadVisible(double marginPx = 40.0);

    // Ruler ticks for current visible range
    std::vector<RulerTick> getVisibleRulerTicks() const;

    // Track layout
    double getDefaultTrackHeight() const { return defaultTrackHeight_; }
    void setDefaultTrackHeight(double h) { defaultTrackHeight_ = h; }

    std::vector<TrackVisualLayout> getTrackLayouts() const;
    std::optional<int> getTrackIndexAtY(double y) const;
    double getTotalTracksHeight() const;

    // Marquee selection
    void startMarquee(double x, double y);
    void updateMarquee(double x, double y);
    void finishMarquee(bool addToExistingSelection = false);
    void cancelMarquee();
    bool isMarqueeActive() const { return isMarqueeActive_; }
    const MarqueeRect& getMarqueeRect() const { return marqueeRect_; }

    // Snapping lines
    std::vector<double> getActiveSnapPixelLines() const;

private:
    EditorEngine& engine_;
    TimelineZoomController zoomController_{100.0};
    double viewportWidth_{1000.0};
    double scrollOffsetX_{0.0};
    double scrollOffsetY_{0.0};
    double defaultTrackHeight_{56.0};

    bool isMarqueeActive_{false};
    MarqueeRect marqueeRect_;
};

} // namespace catchim::editor
