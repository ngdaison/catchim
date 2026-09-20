#include "ResizeController.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

namespace {
constexpr double BASE_TIMELINE_PIXELS_PER_SECOND = 50.0;
constexpr double SNAP_THRESHOLD_PX = 10.0;
}

void ResizeController::onResizeStart(
    ResizeSide side,
    double clientX,
    std::vector<GroupResizeMember> members
) {
    if (session_.has_value()) {
        cancel();
    }

    if (members.empty()) {
        return;
    }

    if (config_.discardPreview) {
        config_.discardPreview();
    }

    session_ = ActiveResizeSession{
        .side = side,
        .startX = clientX,
        .fps = config_.fps,
        .members = std::move(members),
        .result = std::nullopt
    };
}

core::TimelineTime ResizeController::computeSnappedDelta(
    const ActiveResizeSession& session,
    core::TimelineTime rawDeltaTime
) const {
    if (!config_.snappingEnabled || config_.isShiftHeld || !config_.snapPointsProvider) {
        if (config_.onSnapPointChange) {
            config_.onSnapPointChange(std::nullopt);
        }
        return rawDeltaTime;
    }

    const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
    const double maxSnapDistSec = SNAP_THRESHOLD_PX / (BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom);
    const auto snapPoints = config_.snapPointsProvider();

    std::optional<core::TimelineTime> closestSnapPoint = std::nullopt;
    double closestDist = maxSnapDistSec;
    core::TimelineTime deltaTime = rawDeltaTime;

    for (const auto& member : session.members) {
        const core::TimelineTime baseEdgeTime = (session.side == ResizeSide::Left)
            ? member.startTime
            : (member.startTime + member.duration);
        const core::TimelineTime targetTime = baseEdgeTime + rawDeltaTime;

        for (const auto& sp : snapPoints) {
            const double dist = std::abs((sp - targetTime).toSeconds());
            if (dist < closestDist) {
                closestDist = dist;
                closestSnapPoint = sp;
                deltaTime = sp - baseEdgeTime;
            }
        }
    }

    if (config_.onSnapPointChange) {
        config_.onSnapPointChange(closestSnapPoint);
    }
    return deltaTime;
}

void ResizeController::handleMouseMove(double clientX) {
    if (!session_.has_value()) {
        return;
    }

    const double effectiveZoom = config_.zoomLevel > 0.0 ? config_.zoomLevel : 1.0;
    const double pixelsPerSecond = BASE_TIMELINE_PIXELS_PER_SECOND * effectiveZoom;
    const double rawSeconds = (clientX - session_->startX) / pixelsPerSecond;
    const auto rawDelta = core::TimelineTime::fromSeconds(rawSeconds);
    const auto snappedDelta = computeSnappedDelta(*session_, rawDelta);

    auto result = GroupResizeEngine::computeGroupResize(
        session_->members,
        session_->side,
        snappedDelta,
        session_->fps
    );

    session_->result = result;

    if (config_.previewElements) {
        config_.previewElements(result.updates);
    }
}

void ResizeController::handleMouseUp() {
    if (!session_.has_value()) {
        return;
    }

    if (config_.discardPreview) {
        config_.discardPreview();
    }

    if (session_->result.has_value() && hasResizeChanges(session_->members, *session_->result)) {
        if (config_.commitElements) {
            config_.commitElements(session_->result->updates);
        }
    }

    cancel();
}

void ResizeController::cancel() {
    if (config_.discardPreview) {
        config_.discardPreview();
    }
    if (config_.onSnapPointChange) {
        config_.onSnapPointChange(std::nullopt);
    }
    session_.reset();
}

bool ResizeController::hasResizeChanges(
    const std::vector<GroupResizeMember>& members,
    const GroupResizeResult& result
) noexcept {
    for (const auto& update : result.updates) {
        auto it = std::find_if(members.begin(), members.end(), [&](const GroupResizeMember& m) {
            return m.elementId == update.elementId && m.trackId == update.trackId;
        });
        if (it != members.end()) {
            if (it->trimStart != update.patch.trimStart ||
                it->trimEnd != update.patch.trimEnd ||
                it->startTime != update.patch.startTime ||
                it->duration != update.patch.duration) {
                return true;
            }
        }
    }
    return false;
}

} // namespace catchim::editor
