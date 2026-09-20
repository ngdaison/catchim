#include "CanvasViewportController.h"
#include <numbers>

namespace catchim::editor {

// ============================================================================
// ElementBounds
// ============================================================================

Vec2D ElementBounds::getCornerPosition(BoundsCorner corner) const noexcept {
    double halfW = width / 2.0;
    double halfH = height / 2.0;
    double angleRad = (rotation * std::numbers::pi) / 180.0;
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);

    double localX = (corner == BoundsCorner::TopLeft || corner == BoundsCorner::BottomLeft) ? -halfW : halfW;
    double localY = (corner == BoundsCorner::TopLeft || corner == BoundsCorner::TopRight) ? -halfH : halfH;

    return Vec2D{
        cx + (localX * c - localY * s),
        cy + (localX * s + localY * c)
    };
}

Vec2D ElementBounds::getEdgeHandlePosition(BoundsEdge edge) const noexcept {
    double halfW = width / 2.0;
    double halfH = height / 2.0;
    double angleRad = (rotation * std::numbers::pi) / 180.0;
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);

    double localX = (edge == BoundsEdge::Right) ? halfW : (edge == BoundsEdge::Left ? -halfW : 0.0);
    double localY = (edge == BoundsEdge::Bottom) ? halfH : 0.0;

    return Vec2D{
        cx + (localX * c - localY * s),
        cy + (localX * s + localY * c)
    };
}

// ============================================================================
// CanvasViewportController Static Helpers
// ============================================================================

double CanvasViewportController::getFitScale(
    double canvasWidth,
    double canvasHeight,
    double viewportWidth,
    double viewportHeight
) noexcept {
    if (canvasHeight <= 0.0 || canvasWidth <= 0.0 || viewportHeight <= 0.0 || viewportWidth <= 0.0) {
        return 1.0;
    }
    return std::min(viewportWidth / canvasWidth, viewportHeight / canvasHeight);
}

double CanvasViewportController::clampZoom(double zoom) noexcept {
    return std::clamp(zoom, PREVIEW_ZOOM_MIN, PREVIEW_ZOOM_MAX);
}

double CanvasViewportController::getClampedCenterAxis(
    double center,
    double axisSize,
    double scale,
    double viewportSize
) noexcept {
    if (scale <= 0.0) {
        return axisSize / 2.0;
    }
    double visibleHalfSpan = viewportSize / (2.0 * scale);
    if (visibleHalfSpan >= axisSize / 2.0) {
        return axisSize / 2.0;
    }
    return std::clamp(center, visibleHalfSpan, axisSize - visibleHalfSpan);
}

Vec2D CanvasViewportController::clampViewportCenter(
    const Vec2D& center,
    double canvasWidth,
    double canvasHeight,
    double scale,
    double viewportWidth,
    double viewportHeight
) noexcept {
    return Vec2D{
        getClampedCenterAxis(center.x, canvasWidth, scale, viewportWidth),
        getClampedCenterAxis(center.y, canvasHeight, scale, viewportHeight)
    };
}

Vec2D CanvasViewportController::getCanvasOrigin(const PreviewViewportGeometry& geom) noexcept {
    return Vec2D{
        geom.viewportWidth / 2.0 - geom.centerX * geom.scale,
        geom.viewportHeight / 2.0 - geom.centerY * geom.scale
    };
}

Vec2D CanvasViewportController::screenToCanvas(
    double clientX,
    double clientY,
    double viewportLeft,
    double viewportTop,
    const PreviewViewportGeometry& geom
) noexcept {
    double overlayX = clientX - viewportLeft;
    double overlayY = clientY - viewportTop;
    double s = (geom.scale > 0.0) ? geom.scale : 1.0;

    return Vec2D{
        geom.centerX + (overlayX - geom.viewportWidth / 2.0) / s,
        geom.centerY + (overlayY - geom.viewportHeight / 2.0) / s
    };
}

Vec2D CanvasViewportController::canvasToOverlay(
    double canvasX,
    double canvasY,
    const PreviewViewportGeometry& geom
) noexcept {
    Vec2D origin = getCanvasOrigin(geom);
    return Vec2D{
        origin.x + canvasX * geom.scale,
        origin.y + canvasY * geom.scale
    };
}

Vec2D CanvasViewportController::positionToOverlay(
    double posX,
    double posY,
    const PreviewViewportGeometry& geom
) noexcept {
    return canvasToOverlay(geom.canvasWidth / 2.0 + posX, geom.canvasHeight / 2.0 + posY, geom);
}

Vec2D CanvasViewportController::screenPixelsToLogicalThreshold(double screenPixels, double scale) noexcept {
    double s = (scale > 0.0) ? scale : 1.0;
    return Vec2D{screenPixels / s, screenPixels / s};
}

// ============================================================================
// CanvasViewportController Instance
// ============================================================================

CanvasViewportController::CanvasViewportController(
    double canvasWidth,
    double canvasHeight,
    double viewportWidth,
    double viewportHeight
)
    : canvasWidth_(canvasWidth)
    , canvasHeight_(canvasHeight)
    , viewportWidth_(viewportWidth)
    , viewportHeight_(viewportHeight)
    , zoom_(1.0)
    , center_{canvasWidth / 2.0, canvasHeight / 2.0}
{
}

void CanvasViewportController::setCanvasSize(double width, double height) {
    canvasWidth_ = width;
    canvasHeight_ = height;
    setCenter(center_);
}

void CanvasViewportController::setViewportSize(double width, double height) {
    viewportWidth_ = width;
    viewportHeight_ = height;
    setCenter(center_);
}

void CanvasViewportController::setZoom(double zoom) {
    zoom_ = clampZoom(zoom);
    setCenter(center_);
}

double CanvasViewportController::fitScale() const noexcept {
    return getFitScale(canvasWidth_, canvasHeight_, viewportWidth_, viewportHeight_);
}

double CanvasViewportController::viewportScale() const noexcept {
    return fitScale() * zoom_;
}

void CanvasViewportController::setCenter(const Vec2D& center) {
    center_ = clampViewportCenter(
        center,
        canvasWidth_,
        canvasHeight_,
        viewportScale(),
        viewportWidth_,
        viewportHeight_
    );
}

void CanvasViewportController::zoomIn() {
    setZoom(zoom_ * PREVIEW_ZOOM_STEP);
}

void CanvasViewportController::zoomOut() {
    setZoom(zoom_ / PREVIEW_ZOOM_STEP);
}

void CanvasViewportController::scaleZoom(double factor) {
    setZoom(zoom_ * factor);
}

void CanvasViewportController::fitToScreen() {
    zoom_ = 1.0;
    center_ = Vec2D{canvasWidth_ / 2.0, canvasHeight_ / 2.0};
}

void CanvasViewportController::resetPan() {
    center_ = Vec2D{canvasWidth_ / 2.0, canvasHeight_ / 2.0};
}

void CanvasViewportController::setActualSize() {
    double fs = fitScale();
    setZoom(fs > 0.0 ? 1.0 / fs : 1.0);
}

void CanvasViewportController::setViewportPercent(double percent) {
    double fs = fitScale();
    setZoom(fs > 0.0 ? (percent / 100.0) / fs : 1.0);
}

void CanvasViewportController::panByScreenDelta(double deltaX, double deltaY) {
    if (zoom_ <= 1.0 || (deltaX == 0.0 && deltaY == 0.0)) {
        return;
    }
    double vs = viewportScale();
    setCenter(Vec2D{center_.x + deltaX / vs, center_.y + deltaY / vs});
}

PreviewViewportGeometry CanvasViewportController::getGeometry() const noexcept {
    return PreviewViewportGeometry{
        .canvasWidth = canvasWidth_,
        .canvasHeight = canvasHeight_,
        .centerX = center_.x,
        .centerY = center_.y,
        .scale = viewportScale(),
        .viewportWidth = viewportWidth_,
        .viewportHeight = viewportHeight_
    };
}

Vec2D CanvasViewportController::screenToCanvas(
    double clientX,
    double clientY,
    double viewportLeft,
    double viewportTop
) const noexcept {
    return screenToCanvas(clientX, clientY, viewportLeft, viewportTop, getGeometry());
}

Vec2D CanvasViewportController::canvasToOverlay(double canvasX, double canvasY) const noexcept {
    return canvasToOverlay(canvasX, canvasY, getGeometry());
}

Vec2D CanvasViewportController::positionToOverlay(double posX, double posY) const noexcept {
    return positionToOverlay(posX, posY, getGeometry());
}

} // namespace catchim::editor
