#include "render/PreviewInteractionEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::render {

bool PreviewInteractionEngine::pointInRotatedRect(
    double px, double py,
    double cx, double cy,
    double width, double height,
    double rotationDegrees
) noexcept {
    constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
    double angleRad = rotationDegrees * DEG_TO_RAD;
    double cosVal = std::cos(-angleRad);
    double sinVal = std::sin(-angleRad);
    double dx = px - cx;
    double dy = py - cy;
    double localX = dx * cosVal - dy * sinVal;
    double localY = dx * sinVal + dy * cosVal;
    double halfW = std::abs(width) * 0.5;
    double halfH = std::abs(height) * 0.5;
    return (localX >= -halfW && localX <= halfW && localY >= -halfH && localY <= halfH);
}

std::vector<VisualElementBounds> PreviewInteractionEngine::getHitElements(
    Point2D canvasPos,
    const std::vector<VisualElementBounds>& elementsWithBounds
) {
    std::vector<VisualElementBounds> hits;
    for (int i = static_cast<int>(elementsWithBounds.size()) - 1; i >= 0; --i) {
        const auto& elem = elementsWithBounds[static_cast<size_t>(i)];
        if (pointInRotatedRect(canvasPos.x, canvasPos.y, elem.cx, elem.cy, elem.width, elem.height, elem.rotation)) {
            hits.push_back(elem);
        }
    }
    return hits;
}

const VisualElementBounds* PreviewInteractionEngine::resolvePreferredHit(
    const std::vector<VisualElementBounds>& hits,
    const std::vector<editor::ElementLocation>& preferredElements
) {
    if (preferredElements.empty()) return nullptr;
    for (const auto& hit : hits) {
        for (const auto& pref : preferredElements) {
            if (pref.trackId == hit.trackId && pref.clipId == hit.clipId) {
                return &hit;
            }
        }
    }
    return nullptr;
}

void PreviewInteractionEngine::onPointerDown(
    Point2D canvasPos,
    const std::vector<VisualElementBounds>& visibleElements,
    const std::vector<editor::ElementLocation>& currentSelection
) {
    activePatches_.clear();
    activeSnapLines_.clear();
    dragOrigin_ = canvasPos;
    selection_ = currentSelection;

    auto hits = getHitElements(canvasPos, visibleElements);
    if (!hits.empty()) {
        topmostHit_ = hits.front();
        const auto* pref = resolvePreferredHit(hits, currentSelection);
        selectedHit_ = pref ? std::make_optional(*pref) : std::nullopt;
    } else {
        topmostHit_ = std::nullopt;
        selectedHit_ = std::nullopt;
    }

    gestureKind_ = PreviewGestureKind::Pending;
}

void PreviewInteractionEngine::onPointerMove(
    Point2D canvasPos,
    bool isShiftHeld,
    Size2D canvasSize,
    double snapThresholdPixels
) {
    if (gestureKind_ == PreviewGestureKind::Pending) {
        double dx = std::abs(canvasPos.x - dragOrigin_.x);
        double dy = std::abs(canvasPos.y - dragOrigin_.y);
        if (dx <= MIN_DRAG_DISTANCE && dy <= MIN_DRAG_DISTANCE) {
            activeSnapLines_.clear();
            return;
        }

        const auto* dragTarget = selectedHit_ ? &(*selectedHit_) : (topmostHit_ ? &(*topmostHit_) : nullptr);
        if (!dragTarget) {
            gestureKind_ = PreviewGestureKind::Idle;
            activeSnapLines_.clear();
            return;
        }

        // Check if dragTarget is already selected
        bool isAlreadySelected = std::any_of(selection_.begin(), selection_.end(), [&](const editor::ElementLocation& loc) {
            return loc.trackId == dragTarget->trackId && loc.clipId == dragTarget->clipId;
        });

        if (!isAlreadySelected) {
            selection_ = { editor::ElementLocation{dragTarget->trackId, dragTarget->clipId} };
            draggingElements_ = { *dragTarget };
        } else {
            // Drag the selected target first, plus any other selected
            draggingElements_.clear();
            draggingElements_.push_back(*dragTarget);
        }

        dragBoundsSize_ = Size2D{dragTarget->width, dragTarget->height};
        dragRotation_ = dragTarget->rotation;
        gestureKind_ = PreviewGestureKind::Dragging;
    }

    if (gestureKind_ != PreviewGestureKind::Dragging || draggingElements_.empty()) {
        return;
    }

    const auto& firstElement = draggingElements_.front();
    double deltaX = canvasPos.x - dragOrigin_.x;
    double deltaY = canvasPos.y - dragOrigin_.y;

    Point2D proposedPosition{
        firstElement.initialTransform.positionX + deltaX,
        firstElement.initialTransform.positionY + deltaY
    };

    Point2D snappedPosition = proposedPosition;
    activeSnapLines_.clear();

    if (!isShiftHeld) {
        auto snapResult = PreviewSnap::snapPosition(
            proposedPosition,
            canvasSize,
            dragBoundsSize_,
            dragRotation_,
            Point2D{snapThresholdPixels, snapThresholdPixels}
        );
        snappedPosition = snapResult.snappedPosition;
        activeSnapLines_ = std::move(snapResult.activeLines);
    }

    double deltaSnappedX = snappedPosition.x - firstElement.initialTransform.positionX;
    double deltaSnappedY = snappedPosition.y - firstElement.initialTransform.positionY;

    activePatches_.clear();
    for (const auto& elem : draggingElements_) {
        nlohmann::json patch = elem.initialParams;
        patch["transform.positionX"] = elem.initialTransform.positionX + deltaSnappedX;
        patch["transform.positionY"] = elem.initialTransform.positionY + deltaSnappedY;
        activePatches_.push_back(editor::ElementPatch{elem.trackId, elem.clipId, patch});
    }
}

void PreviewInteractionEngine::onPointerUp(bool isCancelled) {
    if (gestureKind_ == PreviewGestureKind::Dragging) {
        if (isCancelled) {
            activePatches_.clear();
        }
        activeSnapLines_.clear();
        gestureKind_ = PreviewGestureKind::Idle;
        return;
    }

    if (gestureKind_ == PreviewGestureKind::Pending) {
        if (!isCancelled) {
            if (topmostHit_) {
                selection_ = { editor::ElementLocation{topmostHit_->trackId, topmostHit_->clipId} };
            } else {
                selection_.clear();
            }
        }
        activeSnapLines_.clear();
        gestureKind_ = PreviewGestureKind::Idle;
    }
}

bool PreviewInteractionEngine::onDoubleClick(
    Point2D canvasPos,
    const std::vector<VisualElementBounds>& visibleElements
) {
    auto hits = getHitElements(canvasPos, visibleElements);
    if (!hits.empty() && hits.front().clipType == "text") {
        const auto& hit = hits.front();
        std::string content;
        if (hit.initialParams.contains("text") && hit.initialParams["text"].is_string()) {
            content = hit.initialParams["text"].get<std::string>();
        }
        textEditingState_ = TextEditingState{hit.trackId, hit.clipId, std::move(content)};
        return true;
    }
    return false;
}

void PreviewInteractionEngine::cancelGesture() {
    activePatches_.clear();
    activeSnapLines_.clear();
    gestureKind_ = PreviewGestureKind::Idle;
}

} // namespace catchim::render
