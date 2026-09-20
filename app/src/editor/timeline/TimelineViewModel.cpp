#include "editor/timeline/TimelineViewModel.h"
#include <algorithm>

namespace catchim::editor {

TimelineViewModel::TimelineViewModel(EditorEngine& engine)
    : engine_(engine)
{
}

void TimelineViewModel::setViewportWidth(double widthPixels) {
    if (widthPixels > 10.0) {
        viewportWidth_ = widthPixels;
    }
}

void TimelineViewModel::setScrollOffsetX(double scrollX) {
    scrollOffsetX_ = std::max(0.0, scrollX);
}

void TimelineViewModel::setScrollOffsetY(double scrollY) {
    scrollOffsetY_ = std::max(0.0, scrollY);
}

TimeRange TimelineViewModel::getVisibleTimeRange() const {
    core::TimelineTime start = pixelToTime(0.0);
    core::TimelineTime end = pixelToTime(viewportWidth_);
    return {start, end};
}

double TimelineViewModel::timeToPixel(core::TimelineTime time) const {
    return zoomController_.timeToPixel(time, scrollOffsetX_);
}

core::TimelineTime TimelineViewModel::pixelToTime(double pixelX) const {
    return zoomController_.pixelToTime(pixelX, scrollOffsetX_);
}

bool TimelineViewModel::ensurePlayheadVisible(double marginPx) {
    double absolutePx = zoomController_.timeToPixel(engine_.playback().currentTime(), 0.0);
    double minVisiblePx = scrollOffsetX_ + marginPx;
    double maxVisiblePx = scrollOffsetX_ + viewportWidth_ - marginPx;

    if (absolutePx < minVisiblePx) {
        scrollOffsetX_ = std::max(0.0, absolutePx - marginPx);
        return true;
    } else if (absolutePx > maxVisiblePx) {
        scrollOffsetX_ = std::max(0.0, absolutePx - viewportWidth_ + marginPx);
        return true;
    }
    return false;
}

std::vector<RulerTick> TimelineViewModel::getVisibleRulerTicks() const {
    TimeRange range = getVisibleTimeRange();
    double zoom = zoomController_.zoomLevel();
    double fps = engine_.project().settings().fps.toFps();

    return RulerEngine::generateRulerTicks(
        range.start,
        range.end,
        zoom,
        fps,
        scrollOffsetX_
    );
}

std::vector<TrackVisualLayout> TimelineViewModel::getTrackLayouts() const {
    std::vector<TrackVisualLayout> layouts;
    const auto* tl = engine_.activeTimeline();
    if (!tl) return layouts;

    auto tracks = tl->allTracks();

    double currentY = 0.0;
    for (size_t i = 0; i < tracks.size(); ++i) {
        const auto* track = tracks[i];
        TrackVisualLayout layout;
        layout.trackIndex = static_cast<int>(i);
        layout.trackId = track->id();
        layout.topY = currentY - scrollOffsetY_;
        layout.height = defaultTrackHeight_;
        layout.isMuted = track->isMuted();
        layout.isLocked = false;

        layouts.push_back(layout);
        currentY += defaultTrackHeight_;
    }

    return layouts;
}

std::optional<int> TimelineViewModel::getTrackIndexAtY(double y) const {
    double absoluteY = y + scrollOffsetY_;
    if (absoluteY < 0.0) return std::nullopt;

    const auto* tl = engine_.activeTimeline();
    if (!tl) return std::nullopt;

    auto tracks = tl->allTracks();
    if (tracks.empty()) return std::nullopt;

    int index = static_cast<int>(absoluteY / defaultTrackHeight_);
    if (index >= 0 && index < static_cast<int>(tracks.size())) {
        return index;
    }
    return std::nullopt;
}

double TimelineViewModel::getTotalTracksHeight() const {
    const auto* tl = engine_.activeTimeline();
    if (!tl) return 0.0;
    return static_cast<double>(tl->allTracks().size()) * defaultTrackHeight_;
}

void TimelineViewModel::startMarquee(double x, double y) {
    isMarqueeActive_ = true;
    marqueeRect_.startX = x;
    marqueeRect_.startY = y;
    marqueeRect_.currentX = x;
    marqueeRect_.currentY = y;
}

void TimelineViewModel::updateMarquee(double x, double y) {
    if (isMarqueeActive_) {
        marqueeRect_.currentX = x;
        marqueeRect_.currentY = y;
    }
}

void TimelineViewModel::finishMarquee(bool addToExistingSelection) {
    if (!isMarqueeActive_) return;
    isMarqueeActive_ = false;

    if (!addToExistingSelection) {
        engine_.deselectAll();
    }

    const auto* tl = engine_.activeTimeline();
    if (!tl) return;

    double minPxX = marqueeRect_.minX();
    double maxPxX = marqueeRect_.maxX();
    double minY = marqueeRect_.minY();
    double maxY = marqueeRect_.maxY();

    core::TimelineTime startTime = pixelToTime(minPxX);
    core::TimelineTime endTime = pixelToTime(maxPxX);

    auto tracks = tl->allTracks();
    for (size_t i = 0; i < tracks.size(); ++i) {
        double trackTopY = static_cast<double>(i) * defaultTrackHeight_ - scrollOffsetY_;
        double trackBottomY = trackTopY + defaultTrackHeight_;

        // Check if track intersects marquee vertically
        if (trackBottomY < minY || trackTopY > maxY) {
            continue;
        }

        const auto* track = tracks[i];
        for (const auto& clip : track->clips()) {
            core::TimelineTime clipStart = clip.startTime();
            core::TimelineTime clipEnd = clipStart + clip.duration();

            // Check if clip intersects horizontally
            if (clipEnd > startTime && clipStart < endTime) {
                engine_.selectClip(clip.id(), true);
            }
        }
    }
}

void TimelineViewModel::cancelMarquee() {
    isMarqueeActive_ = false;
}

std::vector<double> TimelineViewModel::getActiveSnapPixelLines() const {
    std::vector<double> snapLines;
    core::TimelineTime playhead = engine_.playback().currentTime();
    snapLines.push_back(timeToPixel(playhead));
    return snapLines;
}

} // namespace catchim::editor
