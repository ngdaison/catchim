#pragma once

#include "render/HitTesting.h"
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::render {

struct BaseMaskParams {
    double feather{0.0};
    bool inverted{false};
    std::string strokeColor{"#ffffff"};
    double strokeWidth{0.0};
    std::string strokeAlign{"center"}; // "inside" | "center" | "outside"

    bool operator==(const BaseMaskParams& other) const = default;
};

struct RectangleMaskParams : public BaseMaskParams {
    double centerX{0.0};
    double centerY{0.0};
    double width{0.6};
    double height{0.6};
    double rotation{0.0};
    double scale{1.0};

    bool operator==(const RectangleMaskParams& other) const = default;
};

struct SplitMaskParams : public BaseMaskParams {
    double centerX{0.0};
    double centerY{0.0};
    double rotation{0.0};

    bool operator==(const SplitMaskParams& other) const = default;
};

struct BoxLikeGeometry {
    double centerX{0.0};
    double centerY{0.0};
    double maskWidth{0.0};
    double maskHeight{0.0};
    double rotationRad{0.0};
};

struct SplitLineGeometry {
    double normalX{0.0};
    double normalY{0.0};
    double lineX{0.0};
    double lineY{0.0};
};

struct CubicBezierCurve {
    Point2D start;
    Point2D cp1;
    Point2D cp2;
    Point2D end;
};

struct HeartCurves {
    CubicBezierCurve rightBranch;
    CubicBezierCurve leftBranch;
};

/**
 * @brief Geometry generators and interactive parameter calculators for all builtin mask shapes.
 * Corresponds to web/src/masks/builtin/definitions/ and web/src/masks/builtin/box-like.ts.
 */
class BuiltinMaskGeometry {
public:
    static constexpr double DEFAULT_SHAPE_MASK_SHORT_SIDE_RATIO = 0.6;
    static constexpr double MIN_MASK_DIMENSION = 0.01;
    static constexpr double MAX_FEATHER = 1000.0;
    static constexpr double FEATHER_HANDLE_SCALE = 0.11;
    static constexpr double NORMAL_SNAP_EPSILON = 1e-10;
    static constexpr double MIN_POLYGON_AREA_PX = 0.5;
    static constexpr double INTERSECTION_EPSILON = 1e-6;
    static constexpr double STAR_INNER_RADIUS_RATIO = 0.45;
    static constexpr size_t STAR_VERTEX_COUNT = 10;

    // --- Base & Utility ---
    static BaseMaskParams getDefaultBaseMaskParams();
    static double getStrokeOffset(const std::string& strokeAlign, double strokeWidth) noexcept;
    static Point2D rotatePoint(double x, double y, double centerX, double centerY, double rotationRad) noexcept;
    static RectangleMaskParams getDefaultSquareMaskParams(double elementWidth, double elementHeight);
    static BoxLikeGeometry getBoxLikeGeometry(const RectangleMaskParams& params, double width, double height);

    // --- Feather & Param Updates ---
    static double computeFeatherUpdate(
        double startFeather,
        double deltaX,
        double deltaY,
        double directionX,
        double directionY
    ) noexcept;

    static RectangleMaskParams computeBoxMaskParamUpdate(
        const std::string& handleKind,
        const std::string& sideOrCorner, // "left"|"right"|"top"|"bottom" or "top-left" etc.
        const RectangleMaskParams& startParams,
        double deltaX,
        double deltaY,
        double boundsWidth,
        double boundsHeight
    );

    static SplitMaskParams computeSplitMaskParamUpdate(
        const std::string& handleKind, // "position" | "rotation" | "feather"
        const SplitMaskParams& startParams,
        double deltaX,
        double deltaY,
        double startCanvasX,
        double startCanvasY,
        double boundsCx,
        double boundsCy,
        double boundsWidth,
        double boundsHeight,
        double canvasWidth,
        double canvasHeight
    );

    // --- Shape Specific Geometries ---
    static std::vector<Point2D> buildRectangleCorners(
        const RectangleMaskParams& params,
        double width,
        double height,
        bool withStrokeOffset = false
    );

    static BoxLikeGeometry buildEllipseParams(
        const RectangleMaskParams& params,
        double width,
        double height,
        bool withStrokeOffset = false
    );

    static RectangleMaskParams getDefaultCinematicBarsMaskParams(double elementWidth, double elementHeight);

    static std::vector<Point2D> buildCinematicBarsCorners(
        const RectangleMaskParams& params,
        double width,
        double height,
        bool withStrokeOffset = false
    );

    static std::vector<Point2D> buildDiamondPoints(
        const RectangleMaskParams& params,
        double width,
        double height,
        bool withStrokeOffset = false
    );

    static HeartCurves buildHeartCurves(
        const RectangleMaskParams& params,
        double width,
        double height,
        bool withStrokeOffset = false
    );

    static std::vector<Point2D> buildStarVertices(
        const RectangleMaskParams& params,
        double width,
        double height,
        bool withStrokeOffset = false
    );

    // --- Split Mask Line & Clipping ---
    static SplitLineGeometry splitLineGeometry(
        double centerX,
        double centerY,
        double rotation,
        double width,
        double height
    ) noexcept;

    static double halfPlaneSign(
        double lineX,
        double lineY,
        double normalX,
        double normalY,
        double x,
        double y
    ) noexcept;

    static std::optional<Point2D> lineEdgeIntersection(
        double lineX,
        double lineY,
        double normalX,
        double normalY,
        double x1,
        double y1,
        double x2,
        double y2
    ) noexcept;

    static std::optional<std::pair<Point2D, Point2D>> getSplitMaskStrokeSegment(
        const SplitMaskParams& params,
        double width,
        double height
    );

    static std::vector<Point2D> buildSplitPolygonVertices(
        const SplitMaskParams& params,
        double width,
        double height
    );

    static double computePolygonArea(const std::vector<Point2D>& vertices) noexcept;
};

} // namespace catchim::render
