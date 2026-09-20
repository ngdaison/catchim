#pragma once

#include "render/masks/BuiltinMaskGeometry.h"
#include "render/PreviewSnap.h"
#include "editor/canvas/CanvasViewportController.h"
#include <vector>
#include <string>

namespace catchim::render {

using editor::ElementBounds;

struct MaskSnapResult {
    RectangleMaskParams params;
    std::vector<SnapLine> activeLines;
};

struct SplitMaskSnapResult {
    SplitMaskParams params;
    std::vector<SnapLine> activeLines;
};

struct MaskSnapGeometry {
    Point2D position;
    Size2D size;
    double rotation{0.0};
};

/**
 * @brief Mask Snapping engine aligning mask center, edges, corners, and rotation to canvas geometry.
 * Corresponds to web/src/masks/snap.ts and web/src/masks/geometry.ts.
 */
class MaskSnapEngine {
public:
    static std::vector<SnapLine> toGlobalMaskSnapLines(
        const std::vector<SnapLine>& lines,
        const ElementBounds& bounds,
        const Size2D& canvasSize
    );

    static Point2D getMaskLocalCenter(
        double centerX,
        double centerY,
        const ElementBounds& bounds
    ) noexcept;

    static std::pair<double, double> setMaskLocalCenter(
        const Point2D& center,
        const ElementBounds& bounds
    ) noexcept;

    static MaskSnapGeometry getMaskSnapGeometry(
        const RectangleMaskParams& params,
        const ElementBounds& bounds
    ) noexcept;

    static MaskSnapGeometry getSplitMaskSnapGeometry(
        const SplitMaskParams& params,
        const ElementBounds& bounds
    ) noexcept;

    static MaskSnapResult snapBoxMaskInteraction(
        const std::string& handleKind, // "position" | "rotation" | "edge" | "corner" | "scale"
        const std::string& sideOrCorner, // "left"|"right"|"top"|"bottom" or "top-left" etc.
        const RectangleMaskParams& startParams,
        const RectangleMaskParams& proposedParams,
        const ElementBounds& bounds,
        const Size2D& canvasSize,
        Point2D snapThreshold = {PreviewSnap::DEFAULT_SNAP_THRESHOLD, PreviewSnap::DEFAULT_SNAP_THRESHOLD}
    );

    static SplitMaskSnapResult snapSplitMaskInteraction(
        const std::string& handleKind, // "position" | "rotation"
        const SplitMaskParams& proposedParams,
        const ElementBounds& bounds,
        const Size2D& canvasSize,
        Point2D snapThreshold = {PreviewSnap::DEFAULT_SNAP_THRESHOLD, PreviewSnap::DEFAULT_SNAP_THRESHOLD}
    );
};

} // namespace catchim::render
