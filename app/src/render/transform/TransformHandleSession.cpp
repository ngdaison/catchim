#include "TransformHandleSession.h"
#include <cmath>
#include <numbers>
#include <algorithm>

namespace catchim::render {

void TransformHandleSession::startCornerScale(
    editor::BoundsCorner corner,
    Vec2D startCanvasPos,
    const editor::ElementBounds& bounds,
    const Transform& initialTransform
) {
    kind_ = HandleSessionKind::CornerScale;
    corner_ = corner;
    startPos_ = startCanvasPos;
    initialBounds_ = bounds;
    initialTransform_ = initialTransform;
    currentTransform_ = initialTransform;

    double dx = startCanvasPos.x - bounds.cx;
    double dy = startCanvasPos.y - bounds.cy;
    initialDistance_ = std::max(1.0, std::sqrt(dx * dx + dy * dy));
}

void TransformHandleSession::startEdgeScale(
    editor::BoundsEdge edge,
    Vec2D startCanvasPos,
    const editor::ElementBounds& bounds,
    const Transform& initialTransform
) {
    kind_ = HandleSessionKind::EdgeScale;
    edge_ = edge;
    startPos_ = startCanvasPos;
    initialBounds_ = bounds;
    initialTransform_ = initialTransform;
    currentTransform_ = initialTransform;
}

void TransformHandleSession::startRotation(
    Vec2D startCanvasPos,
    const editor::ElementBounds& bounds,
    const Transform& initialTransform
) {
    kind_ = HandleSessionKind::Rotation;
    startPos_ = startCanvasPos;
    initialBounds_ = bounds;
    initialTransform_ = initialTransform;
    currentTransform_ = initialTransform;

    double dx = startCanvasPos.x - bounds.cx;
    double dy = startCanvasPos.y - bounds.cy;
    initialAngleDegrees_ = std::atan2(dy, dx) * 180.0 / std::numbers::pi;
}

HandleTransformResult TransformHandleSession::update(Vec2D currentCanvasPos, bool uniformLock) {
    (void)uniformLock;
    HandleTransformResult result;

    if (kind_ == HandleSessionKind::CornerScale) {
        double dx = currentCanvasPos.x - initialBounds_.cx;
        double dy = currentCanvasPos.y - initialBounds_.cy;
        double curDist = std::sqrt(dx * dx + dy * dy);
        double factor = curDist / initialDistance_;

        double proposedScaleX = initialTransform_.scaleX * factor;
        double proposedScaleY = initialTransform_.scaleY * factor;

        // Snap to initial scale (1.0 or identity) if close within 3%
        bool snapped = false;
        if (std::abs(factor - 1.0) < 0.03) {
            proposedScaleX = initialTransform_.scaleX;
            proposedScaleY = initialTransform_.scaleY;
            snapped = true;
        }

        proposedScaleX = std::max(PreviewSnap::MIN_SCALE, proposedScaleX);
        proposedScaleY = std::max(PreviewSnap::MIN_SCALE, proposedScaleY);

        currentTransform_.scaleX = proposedScaleX;
        currentTransform_.scaleY = proposedScaleY;

        result.transform = currentTransform_;
        result.isSnapped = snapped;
        return result;
    }

    if (kind_ == HandleSessionKind::EdgeScale) {
        double dx = currentCanvasPos.x - initialBounds_.cx;
        double dy = currentCanvasPos.y - initialBounds_.cy;
        double rad = -initialBounds_.rotation * std::numbers::pi / 180.0;
        double c = std::cos(rad);
        double s = std::sin(rad);
        double localX = dx * c - dy * s;
        double localY = dx * s + dy * c;

        if (edge_ == editor::BoundsEdge::Right || edge_ == editor::BoundsEdge::Left) {
            double halfW = std::max(1.0, initialBounds_.width / 2.0);
            double factorX = std::abs(localX) / halfW;
            currentTransform_.scaleX = std::max(PreviewSnap::MIN_SCALE, initialTransform_.scaleX * factorX);
        } else if (edge_ == editor::BoundsEdge::Bottom) {
            double halfH = std::max(1.0, initialBounds_.height / 2.0);
            double factorY = std::abs(localY) / halfH;
            currentTransform_.scaleY = std::max(PreviewSnap::MIN_SCALE, initialTransform_.scaleY * factorY);
        }

        result.transform = currentTransform_;
        return result;
    }

    if (kind_ == HandleSessionKind::Rotation) {
        double dx = currentCanvasPos.x - initialBounds_.cx;
        double dy = currentCanvasPos.y - initialBounds_.cy;
        double curAngle = std::atan2(dy, dx) * 180.0 / std::numbers::pi;
        double deltaAngle = curAngle - initialAngleDegrees_;
        double proposedRot = initialTransform_.rotate + deltaAngle;

        auto snapRes = PreviewSnap::snapRotation(proposedRot);
        currentTransform_.rotate = snapRes.snappedRotation;

        result.transform = currentTransform_;
        result.isSnapped = snapRes.isSnapped;
        return result;
    }

    result.transform = currentTransform_;
    return result;
}

void TransformHandleSession::reset() noexcept {
    kind_ = HandleSessionKind::Idle;
    startPos_ = Vec2D{0.0, 0.0};
    initialDistance_ = 1.0;
    initialAngleDegrees_ = 0.0;
}

} // namespace catchim::render
