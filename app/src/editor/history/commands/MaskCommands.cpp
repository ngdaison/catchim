#include "MaskCommands.h"
#include <algorithm>

namespace catchim::editor {

// ============================================================================
// InsertCustomMaskPointCommand
// ============================================================================

InsertCustomMaskPointCommand::InsertCustomMaskPointCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string maskId,
    size_t segmentIndex,
    double t
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , maskId_(std::move(maskId))
    , segmentIndex_(segmentIndex)
    , t_(t)
{
}

bool InsertCustomMaskPointCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isMaskableElement(*clip)) {
        return false;
    }

    auto& masks = clip->masks();
    auto it = std::find_if(masks.begin(), masks.end(), [&](const MaskInstance& m) {
        return m.id == maskId_ && m.type == "freeform";
    });
    if (it == masks.end()) {
        return false;
    }

    // Parse path points
    std::vector<render::FreeformPathPoint> points;
    if (it->params.contains("path")) {
        if (it->params["path"].is_string()) {
            points = render::FreeformMaskGeometry::parseFreeformPath(it->params["path"].get<std::string>());
        } else if (it->params["path"].is_array()) {
            points = render::FreeformMaskGeometry::parseFreeformPath(it->params["path"].dump());
        }
    }

    bool isClosed = it->params.value("closed", true);
    size_t segCount = render::FreeformMaskGeometry::getFreeformSegmentCount(points, isClosed);
    if (segmentIndex_ >= segCount) {
        return false;
    }

    previousMasks_ = masks;

    insertedPointId_ = render::FreeformMaskGeometry::insertPointOnSegment(points, segmentIndex_, t_, isClosed);
    it->params["path"] = render::FreeformMaskGeometry::serializeFreeformPath(points);
    return true;
}

bool InsertCustomMaskPointCommand::undo() {
    if (!previousMasks_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setMasks(std::move(*previousMasks_));
    previousMasks_.reset();
    return true;
}

// ============================================================================
// DeleteCustomMaskPointsCommand
// ============================================================================

DeleteCustomMaskPointsCommand::DeleteCustomMaskPointsCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string maskId,
    std::vector<std::string> pointIds
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , maskId_(std::move(maskId))
    , pointIds_(std::move(pointIds))
{
}

bool DeleteCustomMaskPointsCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isMaskableElement(*clip)) {
        return false;
    }

    auto& masks = clip->masks();
    auto it = std::find_if(masks.begin(), masks.end(), [&](const MaskInstance& m) {
        return m.id == maskId_ && m.type == "freeform";
    });
    if (it == masks.end()) {
        return false;
    }

    // Parse path points
    std::vector<render::FreeformPathPoint> points;
    if (it->params.contains("path")) {
        if (it->params["path"].is_string()) {
            points = render::FreeformMaskGeometry::parseFreeformPath(it->params["path"].get<std::string>());
        } else if (it->params["path"].is_array()) {
            points = render::FreeformMaskGeometry::parseFreeformPath(it->params["path"].dump());
        }
    }

    size_t originalCount = points.size();
    auto nextPoints = render::FreeformMaskGeometry::removeFreeformPathPoints(points, pointIds_);
    if (nextPoints.size() == originalCount) {
        didDelete_ = false;
        return false;
    }

    previousMasks_ = masks;
    bool wasClosed = it->params.value("closed", true);
    bool nextClosed = render::FreeformMaskGeometry::getFreeformPathClosedStateAfterPointRemoval(wasClosed, nextPoints.size());

    it->params["path"] = render::FreeformMaskGeometry::serializeFreeformPath(nextPoints);
    it->params["closed"] = nextClosed;
    didDelete_ = true;
    return true;
}

bool DeleteCustomMaskPointsCommand::undo() {
    if (!previousMasks_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setMasks(std::move(*previousMasks_));
    previousMasks_.reset();
    return true;
}

// ============================================================================
// RemoveMaskCommand
// ============================================================================

RemoveMaskCommand::RemoveMaskCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string maskId
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , maskId_(std::move(maskId))
{
}

bool RemoveMaskCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }

    auto& masks = clip->masks();
    auto it = std::find_if(masks.begin(), masks.end(), [&](const MaskInstance& m) {
        return m.id == maskId_;
    });
    if (it == masks.end()) {
        return false;
    }

    previousMasks_ = masks;
    masks.erase(it);
    return true;
}

bool RemoveMaskCommand::undo() {
    if (!previousMasks_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setMasks(std::move(*previousMasks_));
    previousMasks_.reset();
    return true;
}

// ============================================================================
// ToggleMaskInvertedCommand
// ============================================================================

ToggleMaskInvertedCommand::ToggleMaskInvertedCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string maskId
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , maskId_(std::move(maskId))
{
}

bool ToggleMaskInvertedCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isMaskableElement(*clip)) {
        return false;
    }

    auto& masks = clip->masks();
    auto it = std::find_if(masks.begin(), masks.end(), [&](const MaskInstance& m) {
        return m.id == maskId_;
    });
    if (it == masks.end()) {
        return false;
    }

    previousMasks_ = masks;
    bool inverted = it->params.value("inverted", false);
    it->params["inverted"] = !inverted;
    return true;
}

bool ToggleMaskInvertedCommand::undo() {
    if (!previousMasks_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setMasks(std::move(*previousMasks_));
    previousMasks_.reset();
    return true;
}

} // namespace catchim::editor
