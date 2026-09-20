#pragma once

#include "render/HitTesting.h"
#include "render/masks/BuiltinMaskGeometry.h"
#include "editor/canvas/CanvasViewportController.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::render {

using editor::ElementBounds;

struct MaskHandle {
    std::string kind; // "icon" | "corner" | "edge" | "point"
    std::string idKind; // "position" | "rotation" | "feather" | "scale" | "edge" | "corner"
    std::string detail; // "left", "right", "top", "bottom", "top-left", etc.
    double x{0.0};
    double y{0.0};
    std::string cursor;
    std::string icon; // "rotate" | "feather"
    std::string edgeAxis; // "horizontal" | "vertical"
    double rotation{0.0};
};

struct MaskLineOverlay {
    std::string id{"line"};
    Point2D start;
    Point2D end;
    std::string cursor{"move"};
};

struct MaskRectOverlay {
    std::string id{"bounding-box"};
    Point2D center;
    double width{0.0};
    double height{0.0};
    double rotation{0.0};
    bool dashed{false};
    std::string cursor{"move"};
};

struct MaskShapeOverlay {
    std::string id{"shape-outline"};
    Point2D center;
    double width{0.0};
    double height{0.0};
    double rotation{0.0};
    std::string pathData;
    std::string cursor{"move"};
};

/**
 * @brief Interactive mask handle positioning and overlay visualizer.
 * Corresponds to web/src/masks/handle-positions.ts.
 */
class MaskInteractionEngine {
public:
    static constexpr double LINE_HANDLE_OFFSET_SCREEN_PX = 20.0;
    static constexpr double BOX_HANDLE_OFFSET_SCREEN_PX = 20.0;
    static constexpr double LINE_EXTENT_MULTIPLIER = 50.0;

    static std::pair<Point2D, Point2D> getLineMaskLinePoints(
        double centerX,
        double centerY,
        double rotation,
        const ElementBounds& bounds
    ) noexcept;

    static MaskLineOverlay getLineMaskOverlay(
        double centerX,
        double centerY,
        double rotation,
        const ElementBounds& bounds,
        const std::string& cursor = "move"
    ) noexcept;

    static std::vector<MaskHandle> getLineMaskHandlePositions(
        double centerX,
        double centerY,
        double rotation,
        double feather,
        const ElementBounds& bounds,
        double displayScale = 1.0
    );

    static std::vector<MaskHandle> getBoxMaskHandlePositions(
        double centerX,
        double centerY,
        double width,
        double height,
        double rotation,
        double feather,
        const std::string& sizeMode, // "width-height" | "height-only" | "width-only" | "uniform" | "none"
        const ElementBounds& bounds,
        double displayScale = 1.0,
        bool showScaleHandle = true
    );

    static MaskRectOverlay getBoxMaskRectOverlay(
        double centerX,
        double centerY,
        double width,
        double height,
        double rotation,
        const ElementBounds& bounds,
        bool dashed = false,
        const std::string& cursor = "move"
    ) noexcept;

    static MaskShapeOverlay getBoxMaskShapeOverlay(
        double centerX,
        double centerY,
        double width,
        double height,
        double rotation,
        const ElementBounds& bounds,
        const std::string& pathData,
        const std::string& cursor = "move"
    ) noexcept;
};

} // namespace catchim::render
