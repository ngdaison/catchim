#include "render/PreviewCoordinateTransformer.h"

namespace catchim::render {

Vec2D PreviewCoordinateTransformer::getCanvasOrigin(const editor::PreviewViewportGeometry& geom) noexcept {
    return editor::CanvasViewportController::getCanvasOrigin(geom);
}

Vec2D PreviewCoordinateTransformer::screenToCanvas(
    double clientX,
    double clientY,
    const editor::PreviewViewportGeometry& geom,
    const Rect2D& viewportRect
) noexcept {
    return editor::CanvasViewportController::screenToCanvas(
        clientX, clientY, viewportRect.x, viewportRect.y, geom
    );
}

Vec2D PreviewCoordinateTransformer::canvasToOverlay(
    double canvasX,
    double canvasY,
    const editor::PreviewViewportGeometry& geom
) noexcept {
    return editor::CanvasViewportController::canvasToOverlay(canvasX, canvasY, geom);
}

Vec2D PreviewCoordinateTransformer::positionToOverlay(
    double positionX,
    double positionY,
    const editor::PreviewViewportGeometry& geom
) noexcept {
    return editor::CanvasViewportController::positionToOverlay(positionX, positionY, geom);
}

Vec2D PreviewCoordinateTransformer::getDisplayScale(const editor::PreviewViewportGeometry& geom) noexcept {
    return Vec2D{geom.scale, geom.scale};
}

Vec2D PreviewCoordinateTransformer::screenPixelsToLogicalThreshold(
    const editor::PreviewViewportGeometry& geom,
    double screenPixels
) noexcept {
    return editor::CanvasViewportController::screenPixelsToLogicalThreshold(screenPixels, geom.scale);
}

} // namespace catchim::render
