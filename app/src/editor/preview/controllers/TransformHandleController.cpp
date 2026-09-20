#include "TransformHandleController.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::editor {

double TransformHandleController::clampScaleNonZero(double scale, double minScale) noexcept {
    if (std::abs(scale) < minScale) {
        return scale < 0.0 ? -minScale : minScale;
    }
    return scale;
}

double TransformHandleController::getCornerDistance(
    double width,
    double height,
    double rotationDeg,
    HandleCorner corner
) noexcept {
    const double halfWidth = width / 2.0;
    const double halfHeight = height / 2.0;
    const double angleRad = (rotationDeg * M_PI) / 180.0;
    const double cosVal = std::cos(angleRad);
    const double sinVal = std::sin(angleRad);

    const double localX = (corner == HandleCorner::TopLeft || corner == HandleCorner::BottomLeft) ? -halfWidth : halfWidth;
    const double localY = (corner == HandleCorner::TopLeft || corner == HandleCorner::TopRight) ? -halfHeight : halfHeight;

    const double rotatedX = localX * cosVal - localY * sinVal;
    const double rotatedY = localX * sinVal + localY * cosVal;

    const double dist = std::sqrt(rotatedX * rotatedX + rotatedY * rotatedY);
    return dist > 0.0 ? dist : 1.0;
}

bool TransformHandleController::shouldClearScaleAnimation(const Clip& clip) noexcept {
    return clip.hasAnimationChannel("transform.scaleX") || clip.hasAnimationChannel("transform.scaleY");
}

void TransformHandleController::clearScaleAnimationChannels(Clip& clip) {
    clip.removeAnimationChannel("transform.scaleX");
    clip.removeAnimationChannel("transform.scaleY");
}

void TransformHandleController::startCornerScale(
    core::TrackId trackId,
    core::ClipId elementId,
    HandleCorner corner,
    double width,
    double height,
    double rotationDeg,
    double initialScaleX,
    double initialScaleY
) {
    kind_ = TransformSessionKind::CornerScale;
    edgeSession_ = std::nullopt;
    rotationSession_ = std::nullopt;
    cornerSession_ = CornerScaleSession{
        .corner = corner,
        .trackId = trackId,
        .elementId = elementId,
        .initialDistance = getCornerDistance(width, height, rotationDeg, corner),
        .baseWidth = width,
        .baseHeight = height,
        .initialScaleX = initialScaleX,
        .initialScaleY = initialScaleY
    };
}

void TransformHandleController::startEdgeScale(
    core::TrackId trackId,
    core::ClipId elementId,
    HandleEdge edge,
    double width,
    double height,
    double rotationDeg,
    double initialScaleX,
    double initialScaleY
) {
    kind_ = TransformSessionKind::EdgeScale;
    cornerSession_ = std::nullopt;
    rotationSession_ = std::nullopt;
    edgeSession_ = EdgeScaleSession{
        .edge = edge,
        .trackId = trackId,
        .elementId = elementId,
        .baseWidth = width,
        .baseHeight = height,
        .rotationRad = (rotationDeg * M_PI) / 180.0,
        .initialScaleX = initialScaleX,
        .initialScaleY = initialScaleY
    };
}

void TransformHandleController::startRotation(
    core::TrackId trackId,
    core::ClipId elementId,
    double pointerAngleRad,
    double initialRotationDeg
) {
    kind_ = TransformSessionKind::Rotation;
    cornerSession_ = std::nullopt;
    edgeSession_ = std::nullopt;
    rotationSession_ = RotationSession{
        .trackId = trackId,
        .elementId = elementId,
        .initialAngle = pointerAngleRad,
        .initialRotation = initialRotationDeg
    };
}

void TransformHandleController::endSession() noexcept {
    kind_ = TransformSessionKind::Idle;
    cornerSession_ = std::nullopt;
    edgeSession_ = std::nullopt;
    rotationSession_ = std::nullopt;
}

} // namespace catchim::editor
