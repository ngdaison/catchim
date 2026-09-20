#pragma once

#include "render/graphics/GraphicGeometry.h"
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>

namespace catchim::editor {

using render::Vec2D;

enum class BoundsCorner {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class BoundsEdge {
    Left,
    Right,
    Bottom
};

inline constexpr double ROTATION_HANDLE_OFFSET = 24.0;
inline constexpr double PREVIEW_ZOOM_MIN = 0.25;
inline constexpr double PREVIEW_ZOOM_MAX = 16.0;
inline constexpr double PREVIEW_ZOOM_STEP = 1.25;

inline constexpr std::array<double, 6> PREVIEW_ZOOM_PRESETS = {
    0.25, 0.50, 0.75, 1.00, 1.50, 2.00
};

struct ElementBounds {
    double cx{0.0};
    double cy{0.0};
    double width{0.0};
    double height{0.0};
    double rotation{0.0}; // in degrees

    Vec2D getCornerPosition(BoundsCorner corner) const noexcept;
    Vec2D getEdgeHandlePosition(BoundsEdge edge) const noexcept;
};

struct PreviewViewportGeometry {
    double canvasWidth{1920.0};
    double canvasHeight{1080.0};
    double centerX{960.0};
    double centerY{540.0};
    double scale{1.0}; // viewportScale = fitScale * zoom
    double viewportWidth{960.0};
    double viewportHeight{540.0};
};

/**
 * @brief Controller managing canvas preview viewport, pan-zoom state, and coordinate conversions.
 * Corresponds to web/src/preview/preview-coords.ts, preview-viewport.tsx, zoom.ts, and element-bounds.ts.
 */
class CanvasViewportController {
public:
    CanvasViewportController(
        double canvasWidth = 1920.0,
        double canvasHeight = 1080.0,
        double viewportWidth = 960.0,
        double viewportHeight = 540.0
    );

    // Static coordinate and scale helpers
    static double getFitScale(
        double canvasWidth,
        double canvasHeight,
        double viewportWidth,
        double viewportHeight
    ) noexcept;

    static double clampZoom(double zoom) noexcept;

    static double getClampedCenterAxis(
        double center,
        double axisSize,
        double scale,
        double viewportSize
    ) noexcept;

    static Vec2D clampViewportCenter(
        const Vec2D& center,
        double canvasWidth,
        double canvasHeight,
        double scale,
        double viewportWidth,
        double viewportHeight
    ) noexcept;

    static Vec2D getCanvasOrigin(const PreviewViewportGeometry& geom) noexcept;

    static Vec2D screenToCanvas(
        double clientX,
        double clientY,
        double viewportLeft,
        double viewportTop,
        const PreviewViewportGeometry& geom
    ) noexcept;

    static Vec2D canvasToOverlay(
        double canvasX,
        double canvasY,
        const PreviewViewportGeometry& geom
    ) noexcept;

    static Vec2D positionToOverlay(
        double posX,
        double posY,
        const PreviewViewportGeometry& geom
    ) noexcept;

    static Vec2D screenPixelsToLogicalThreshold(double screenPixels, double scale) noexcept;

    // Viewport configuration
    void setCanvasSize(double width, double height);
    double canvasWidth() const noexcept { return canvasWidth_; }
    double canvasHeight() const noexcept { return canvasHeight_; }

    void setViewportSize(double width, double height);
    double viewportWidth() const noexcept { return viewportWidth_; }
    double viewportHeight() const noexcept { return viewportHeight_; }

    // Zoom & scale getters/setters
    double zoom() const noexcept { return zoom_; }
    void setZoom(double zoom);

    double fitScale() const noexcept;
    double viewportScale() const noexcept;

    // Center & pan
    Vec2D center() const noexcept { return center_; }
    void setCenter(const Vec2D& center);

    void zoomIn();
    void zoomOut();
    void scaleZoom(double factor);
    void fitToScreen();
    void resetPan();
    void setActualSize();
    void setViewportPercent(double percent);
    void panByScreenDelta(double deltaX, double deltaY);

    // Active geometry snapshot
    PreviewViewportGeometry getGeometry() const noexcept;

    // Instance conversions
    Vec2D screenToCanvas(double clientX, double clientY, double viewportLeft = 0.0, double viewportTop = 0.0) const noexcept;
    Vec2D canvasToOverlay(double canvasX, double canvasY) const noexcept;
    Vec2D positionToOverlay(double posX, double posY) const noexcept;

private:
    double canvasWidth_{1920.0};
    double canvasHeight_{1080.0};
    double viewportWidth_{960.0};
    double viewportHeight_{540.0};
    double zoom_{1.0};
    Vec2D center_{960.0, 540.0};
};

} // namespace catchim::editor
