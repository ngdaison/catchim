#pragma once

#include "editor/canvas/CanvasViewportController.h"
#include "render/HitTesting.h"
#include <cmath>

namespace catchim::render {

using editor::PreviewViewportGeometry;

class PreviewCoordinateTransformer {
public:
    static Vec2D getCanvasOrigin(const editor::PreviewViewportGeometry& geom) noexcept;

    static Vec2D screenToCanvas(
        double clientX,
        double clientY,
        const editor::PreviewViewportGeometry& geom,
        const Rect2D& viewportRect
    ) noexcept;

    static Vec2D canvasToOverlay(
        double canvasX,
        double canvasY,
        const editor::PreviewViewportGeometry& geom
    ) noexcept;

    static Vec2D positionToOverlay(
        double positionX,
        double positionY,
        const editor::PreviewViewportGeometry& geom
    ) noexcept;

    static Vec2D getDisplayScale(const editor::PreviewViewportGeometry& geom) noexcept;

    static Vec2D screenPixelsToLogicalThreshold(
        const editor::PreviewViewportGeometry& geom,
        double screenPixels
    ) noexcept;
};

} // namespace catchim::render
