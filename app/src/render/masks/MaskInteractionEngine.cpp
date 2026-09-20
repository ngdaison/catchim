#include "render/masks/MaskInteractionEngine.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

std::pair<Point2D, Point2D> MaskInteractionEngine::getLineMaskLinePoints(
    double centerX,
    double centerY,
    double rotation,
    const ElementBounds& bounds
) noexcept {
    double angleRad = (rotation * M_PI) / 180.0;
    double normalX = std::cos(angleRad);
    double normalY = std::sin(angleRad);
    double lineDirX = -normalY;
    double lineDirY = normalX;

    double cx = bounds.cx + centerX * bounds.width;
    double cy = bounds.cy + centerY * bounds.height;

    double extent = std::max(bounds.width, bounds.height) * LINE_EXTENT_MULTIPLIER;

    return {
        Point2D{ .x = cx - lineDirX * extent, .y = cy - lineDirY * extent },
        Point2D{ .x = cx + lineDirX * extent, .y = cy + lineDirY * extent }
    };
}

MaskLineOverlay MaskInteractionEngine::getLineMaskOverlay(
    double centerX,
    double centerY,
    double rotation,
    const ElementBounds& bounds,
    const std::string& cursor
) noexcept {
    auto pts = getLineMaskLinePoints(centerX, centerY, rotation, bounds);
    return MaskLineOverlay{
        .id = "line",
        .start = pts.first,
        .end = pts.second,
        .cursor = cursor
    };
}

std::vector<MaskHandle> MaskInteractionEngine::getLineMaskHandlePositions(
    double centerX,
    double centerY,
    double rotation,
    double feather,
    const ElementBounds& bounds,
    double displayScale
) {
    double angleRad = (rotation * M_PI) / 180.0;
    double normalX = std::cos(angleRad);
    double normalY = std::sin(angleRad);

    double cx = bounds.cx + centerX * bounds.width;
    double cy = bounds.cy + centerY * bounds.height;

    double scale = (displayScale <= 0.0) ? 1.0 : displayScale;
    double iconOffsetCanvas = LINE_HANDLE_OFFSET_SCREEN_PX / scale;
    double featherOffset = iconOffsetCanvas + feather * BuiltinMaskGeometry::FEATHER_HANDLE_SCALE;

    std::vector<MaskHandle> handles;
    handles.reserve(2);

    handles.push_back(MaskHandle{
        .kind = "icon",
        .idKind = "rotation",
        .detail = "",
        .x = cx + normalX * iconOffsetCanvas,
        .y = cy + normalY * iconOffsetCanvas,
        .cursor = "crosshair",
        .icon = "rotate",
        .edgeAxis = "",
        .rotation = rotation
    });

    handles.push_back(MaskHandle{
        .kind = "icon",
        .idKind = "feather",
        .detail = "",
        .x = cx - normalX * featherOffset,
        .y = cy - normalY * featherOffset,
        .cursor = "ew-resize",
        .icon = "feather",
        .edgeAxis = "",
        .rotation = rotation
    });

    return handles;
}

std::vector<MaskHandle> MaskInteractionEngine::getBoxMaskHandlePositions(
    double centerX,
    double centerY,
    double width,
    double height,
    double rotation,
    double feather,
    const std::string& sizeMode,
    const ElementBounds& bounds,
    double displayScale,
    bool showScaleHandle
) {
    double cx = bounds.cx + centerX * bounds.width;
    double cy = bounds.cy + centerY * bounds.height;
    double angleRad = (rotation * M_PI) / 180.0;
    double halfWidth = (width * bounds.width) / 2.0;
    double halfHeight = (height * bounds.height) / 2.0;

    double scale = (displayScale <= 0.0) ? 1.0 : displayScale;
    double handleOffsetCanvas = BOX_HANDLE_OFFSET_SCREEN_PX / scale;

    std::vector<MaskHandle> handles;

    // Rotation handle (top)
    Point2D rotPt = BuiltinMaskGeometry::rotatePoint(
        0.0,
        -halfHeight - handleOffsetCanvas,
        0.0,
        0.0,
        angleRad
    );
    handles.push_back(MaskHandle{
        .kind = "icon",
        .idKind = "rotation",
        .detail = "",
        .x = cx + rotPt.x,
        .y = cy + rotPt.y,
        .cursor = "crosshair",
        .icon = "rotate",
        .edgeAxis = "",
        .rotation = rotation
    });

    // Feather handle (bottom)
    Point2D featherPt = BuiltinMaskGeometry::rotatePoint(
        0.0,
        halfHeight + handleOffsetCanvas + feather * BuiltinMaskGeometry::FEATHER_HANDLE_SCALE,
        0.0,
        0.0,
        angleRad
    );
    handles.push_back(MaskHandle{
        .kind = "icon",
        .idKind = "feather",
        .detail = "",
        .x = cx + featherPt.x,
        .y = cy + featherPt.y,
        .cursor = "ns-resize",
        .icon = "feather",
        .edgeAxis = "",
        .rotation = rotation
    });

    auto addRotatedHandle = [&](
        double localX,
        double localY,
        const std::string& kind,
        const std::string& idKind,
        const std::string& detail,
        const std::string& cursor,
        const std::string& edgeAxis
    ) {
        Point2D pt = BuiltinMaskGeometry::rotatePoint(localX, localY, 0.0, 0.0, angleRad);
        handles.push_back(MaskHandle{
            .kind = kind,
            .idKind = idKind,
            .detail = detail,
            .x = cx + pt.x,
            .y = cy + pt.y,
            .cursor = cursor,
            .icon = "",
            .edgeAxis = edgeAxis,
            .rotation = rotation
        });
    };

    if (sizeMode == "width-height") {
        // 4 corners
        addRotatedHandle(-halfWidth, -halfHeight, "corner", "corner", "top-left", "nwse-resize", "");
        addRotatedHandle(halfWidth, -halfHeight, "corner", "corner", "top-right", "nwse-resize", "");
        addRotatedHandle(halfWidth, halfHeight, "corner", "corner", "bottom-right", "nwse-resize", "");
        addRotatedHandle(-halfWidth, halfHeight, "corner", "corner", "bottom-left", "nwse-resize", "");

        // 3 edges
        addRotatedHandle(-halfWidth, 0.0, "edge", "edge", "left", "ew-resize", "horizontal");
        addRotatedHandle(halfWidth, 0.0, "edge", "edge", "right", "ew-resize", "horizontal");
        addRotatedHandle(0.0, halfHeight, "edge", "edge", "bottom", "ns-resize", "vertical");
    } else if (sizeMode == "height-only") {
        // 2 edges
        addRotatedHandle(0.0, -halfHeight, "edge", "edge", "top", "ns-resize", "vertical");
        addRotatedHandle(0.0, halfHeight, "edge", "edge", "bottom", "ns-resize", "vertical");
    } else if (sizeMode == "width-only") {
        // 2 edges
        addRotatedHandle(-halfWidth, 0.0, "edge", "edge", "left", "ew-resize", "horizontal");
        addRotatedHandle(halfWidth, 0.0, "edge", "edge", "right", "ew-resize", "horizontal");
    } else if (sizeMode == "uniform" && showScaleHandle) {
        // 1 corner scale handle
        addRotatedHandle(halfWidth, halfHeight, "corner", "scale", "scale", "nwse-resize", "");
    }

    return handles;
}

MaskRectOverlay MaskInteractionEngine::getBoxMaskRectOverlay(
    double centerX,
    double centerY,
    double width,
    double height,
    double rotation,
    const ElementBounds& bounds,
    bool dashed,
    const std::string& cursor
) noexcept {
    return MaskRectOverlay{
        .id = "bounding-box",
        .center = Point2D{
            .x = bounds.cx + centerX * bounds.width,
            .y = bounds.cy + centerY * bounds.height
        },
        .width = width * bounds.width,
        .height = height * bounds.height,
        .rotation = rotation,
        .dashed = dashed,
        .cursor = cursor
    };
}

MaskShapeOverlay MaskInteractionEngine::getBoxMaskShapeOverlay(
    double centerX,
    double centerY,
    double width,
    double height,
    double rotation,
    const ElementBounds& bounds,
    const std::string& pathData,
    const std::string& cursor
) noexcept {
    return MaskShapeOverlay{
        .id = "shape-outline",
        .center = Point2D{
            .x = bounds.cx + centerX * bounds.width,
            .y = bounds.cy + centerY * bounds.height
        },
        .width = width * bounds.width,
        .height = height * bounds.height,
        .rotation = rotation,
        .pathData = pathData,
        .cursor = cursor
    };
}

} // namespace catchim::render
