#include "render/masks/BuiltinMaskGeometry.h"
#include <algorithm>

namespace catchim::render {

BaseMaskParams BuiltinMaskGeometry::getDefaultBaseMaskParams() {
    return BaseMaskParams{
        .feather = 0.0,
        .inverted = false,
        .strokeColor = "#ffffff",
        .strokeWidth = 0.0,
        .strokeAlign = "center"
    };
}

double BuiltinMaskGeometry::getStrokeOffset(const std::string& strokeAlign, double strokeWidth) noexcept {
    if (strokeAlign == "inside") {
        return -(strokeWidth / 2.0);
    }
    if (strokeAlign == "outside") {
        return strokeWidth / 2.0;
    }
    return 0.0;
}

Point2D BuiltinMaskGeometry::rotatePoint(
    double x,
    double y,
    double centerX,
    double centerY,
    double rotationRad
) noexcept {
    double dx = x - centerX;
    double dy = y - centerY;
    double c = std::cos(rotationRad);
    double s = std::sin(rotationRad);
    return Point2D{
        .x = centerX + dx * c - dy * s,
        .y = centerY + dx * s + dy * c
    };
}

RectangleMaskParams BuiltinMaskGeometry::getDefaultSquareMaskParams(double elementWidth, double elementHeight) {
    double absWidth = std::abs(elementWidth);
    double absHeight = std::abs(elementHeight);
    double shortSide = std::min(absWidth, absHeight);
    double squareSide = (shortSide > 0.0) ? (shortSide * DEFAULT_SHAPE_MASK_SHORT_SIDE_RATIO) : 0.0;
    double w = (absWidth > 0.0) ? (squareSide / absWidth) : DEFAULT_SHAPE_MASK_SHORT_SIDE_RATIO;
    double h = (absHeight > 0.0) ? (squareSide / absHeight) : DEFAULT_SHAPE_MASK_SHORT_SIDE_RATIO;

    RectangleMaskParams params;
    static_cast<BaseMaskParams&>(params) = getDefaultBaseMaskParams();
    params.centerX = 0.0;
    params.centerY = 0.0;
    params.width = w;
    params.height = h;
    params.rotation = 0.0;
    params.scale = 1.0;
    return params;
}

BoxLikeGeometry BuiltinMaskGeometry::getBoxLikeGeometry(
    const RectangleMaskParams& params,
    double width,
    double height
) {
    return BoxLikeGeometry{
        .centerX = width / 2.0 + params.centerX * width,
        .centerY = height / 2.0 + params.centerY * height,
        .maskWidth = std::max(params.width, MIN_MASK_DIMENSION) * width,
        .maskHeight = std::max(params.height, MIN_MASK_DIMENSION) * height,
        .rotationRad = (params.rotation * M_PI) / 180.0
    };
}

double BuiltinMaskGeometry::computeFeatherUpdate(
    double startFeather,
    double deltaX,
    double deltaY,
    double directionX,
    double directionY
) noexcept {
    double projection = deltaX * directionX + deltaY * directionY;
    double val = std::round(startFeather + projection / FEATHER_HANDLE_SCALE);
    return std::clamp(val, 0.0, MAX_FEATHER);
}

RectangleMaskParams BuiltinMaskGeometry::computeBoxMaskParamUpdate(
    const std::string& handleKind,
    const std::string& sideOrCorner,
    const RectangleMaskParams& startParams,
    double deltaX,
    double deltaY,
    double boundsWidth,
    double boundsHeight
) {
    RectangleMaskParams res = startParams;
    if (boundsWidth <= 0.0 || boundsHeight <= 0.0) {
        return res;
    }

    if (handleKind == "position") {
        res.centerX = startParams.centerX + deltaX / boundsWidth;
        res.centerY = startParams.centerY + deltaY / boundsHeight;
        return res;
    }

    if (handleKind == "rotation") {
        double currentAngle = (std::atan2(deltaY, deltaX) * 180.0) / M_PI;
        double newRotation = std::fmod(startParams.rotation + currentAngle, 360.0);
        if (newRotation < 0.0) {
            newRotation += 360.0;
        }
        res.rotation = newRotation;
        return res;
    }

    if (handleKind == "feather") {
        double angleRad = (startParams.rotation * M_PI) / 180.0;
        res.feather = computeFeatherUpdate(
            startParams.feather,
            deltaX,
            deltaY,
            -std::sin(angleRad),
            std::cos(angleRad)
        );
        return res;
    }

    double halfWidth = startParams.width * boundsWidth;
    double halfHeight = startParams.height * boundsHeight;

    if (handleKind == "edge") {
        if (sideOrCorner == "right" || sideOrCorner == "left") {
            double sign = (sideOrCorner == "right") ? 1.0 : -1.0;
            res.width = std::max(
                MIN_MASK_DIMENSION,
                startParams.width + (sign * deltaX * 2.0) / boundsWidth
            );
            return res;
        }
        if (sideOrCorner == "bottom" || sideOrCorner == "top") {
            double sign = (sideOrCorner == "bottom") ? 1.0 : -1.0;
            res.height = std::max(
                MIN_MASK_DIMENSION,
                startParams.height + (sign * deltaY * 2.0) / boundsHeight
            );
            return res;
        }
    }

    if (handleKind == "corner") {
        double signX = (sideOrCorner.find("right") != std::string::npos) ? 1.0 : -1.0;
        double signY = (sideOrCorner.find("bottom") != std::string::npos) ? 1.0 : -1.0;
        double distance = std::sqrt(
            std::pow(signX * deltaX + halfWidth, 2.0) +
            std::pow(signY * deltaY + halfHeight, 2.0)
        );
        double originalDistance = std::sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        double scale = (originalDistance > 0.0) ? (distance / originalDistance) : 1.0;
        res.width = std::max(MIN_MASK_DIMENSION, startParams.width * scale);
        res.height = std::max(MIN_MASK_DIMENSION, startParams.height * scale);
        return res;
    }

    if (handleKind == "scale") {
        double distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);
        double originalDistance = std::sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        double scale = (originalDistance > 0.0) ? (1.0 + distance / originalDistance) : 1.0;
        res.scale = std::max(MIN_MASK_DIMENSION, startParams.scale * scale);
        return res;
    }

    return res;
}

SplitMaskParams BuiltinMaskGeometry::computeSplitMaskParamUpdate(
    const std::string& handleKind,
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
) {
    SplitMaskParams res = startParams;
    if (boundsWidth <= 0.0 || boundsHeight <= 0.0) {
        return res;
    }

    if (handleKind == "position") {
        double rawX = startParams.centerX + deltaX / boundsWidth;
        double rawY = startParams.centerY + deltaY / boundsHeight;

        double minX = -boundsCx / boundsWidth;
        double maxX = (canvasWidth - boundsCx) / boundsWidth;
        double minY = -boundsCy / boundsHeight;
        double maxY = (canvasHeight - boundsCy) / boundsHeight;

        res.centerX = std::clamp(rawX, minX, maxX);
        res.centerY = std::clamp(rawY, minY, maxY);
        return res;
    }

    if (handleKind == "feather") {
        double angleRad = (startParams.rotation * M_PI) / 180.0;
        res.feather = computeFeatherUpdate(
            startParams.feather,
            deltaX,
            deltaY,
            -std::cos(angleRad),
            -std::sin(angleRad)
        );
        return res;
    }

    if (handleKind == "rotation") {
        double pivotX = boundsCx + startParams.centerX * boundsWidth;
        double pivotY = boundsCy + startParams.centerY * boundsHeight;

        double startAngle = (std::atan2(startCanvasY - pivotY, startCanvasX - pivotX) * 180.0) / M_PI;
        double currentAngle = (std::atan2(startCanvasY + deltaY - pivotY, startCanvasX + deltaX - pivotX) * 180.0) / M_PI;

        double deltaAngle = currentAngle - startAngle;
        if (deltaAngle > 180.0) deltaAngle -= 360.0;
        if (deltaAngle < -180.0) deltaAngle += 360.0;

        double newRot = std::fmod(std::fmod(startParams.rotation + deltaAngle, 360.0) + 360.0, 360.0);
        res.rotation = newRot;
        return res;
    }

    return res;
}

std::vector<Point2D> BuiltinMaskGeometry::buildRectangleCorners(
    const RectangleMaskParams& params,
    double width,
    double height,
    bool withStrokeOffset
) {
    auto geom = getBoxLikeGeometry(params, width, height);
    double halfW = geom.maskWidth / 2.0;
    double halfH = geom.maskHeight / 2.0;

    if (withStrokeOffset) {
        double offset = getStrokeOffset(params.strokeAlign, params.strokeWidth);
        halfW = std::max(1.0, halfW + offset);
        halfH = std::max(1.0, halfH + offset);
    }

    std::vector<Point2D> corners = {
        { geom.centerX - halfW, geom.centerY - halfH },
        { geom.centerX + halfW, geom.centerY - halfH },
        { geom.centerX + halfW, geom.centerY + halfH },
        { geom.centerX - halfW, geom.centerY + halfH }
    };

    for (auto& pt : corners) {
        pt = rotatePoint(pt.x, pt.y, geom.centerX, geom.centerY, geom.rotationRad);
    }

    return corners;
}

BoxLikeGeometry BuiltinMaskGeometry::buildEllipseParams(
    const RectangleMaskParams& params,
    double width,
    double height,
    bool withStrokeOffset
) {
    auto geom = getBoxLikeGeometry(params, width, height);
    if (withStrokeOffset) {
        double offset = getStrokeOffset(params.strokeAlign, params.strokeWidth);
        geom.maskWidth = std::max(2.0, geom.maskWidth + 2.0 * offset);
        geom.maskHeight = std::max(2.0, geom.maskHeight + 2.0 * offset);
    }
    return geom;
}

RectangleMaskParams BuiltinMaskGeometry::getDefaultCinematicBarsMaskParams(
    double elementWidth,
    double elementHeight
) {
    double absWidth = std::abs(elementWidth);
    double absHeight = std::abs(elementHeight);
    double diagonal = (absWidth > 0.0 && absHeight > 0.0)
        ? std::sqrt(absWidth * absWidth + absHeight * absHeight)
        : 0.0;
    double fullSpanWidth = (absWidth > 0.0) ? (diagonal / absWidth) : std::sqrt(2.0);

    RectangleMaskParams params;
    static_cast<BaseMaskParams&>(params) = getDefaultBaseMaskParams();
    params.centerX = 0.0;
    params.centerY = 0.0;
    params.width = std::max(fullSpanWidth, 1.0);
    params.height = 0.6;
    params.rotation = 0.0;
    params.scale = 1.0;
    return params;
}

std::vector<Point2D> BuiltinMaskGeometry::buildCinematicBarsCorners(
    const RectangleMaskParams& params,
    double width,
    double height,
    bool withStrokeOffset
) {
    double centerX = width / 2.0 + params.centerX * width;
    double centerY = height / 2.0 + params.centerY * height;
    double maskWidth = std::max(params.width * width, width);
    double maskHeight = std::max(params.height, 0.01) * height;
    double rotationRad = (params.rotation * M_PI) / 180.0;

    double halfW = maskWidth / 2.0;
    double halfH = maskHeight / 2.0;

    if (withStrokeOffset) {
        double offset = getStrokeOffset(params.strokeAlign, params.strokeWidth);
        halfW = std::max(1.0, halfW + offset);
        halfH = std::max(1.0, halfH + offset);
    }

    std::vector<Point2D> corners = {
        { centerX - halfW, centerY - halfH },
        { centerX + halfW, centerY - halfH },
        { centerX + halfW, centerY + halfH },
        { centerX - halfW, centerY + halfH }
    };

    for (auto& pt : corners) {
        pt = rotatePoint(pt.x, pt.y, centerX, centerY, rotationRad);
    }

    return corners;
}

std::vector<Point2D> BuiltinMaskGeometry::buildDiamondPoints(
    const RectangleMaskParams& params,
    double width,
    double height,
    bool withStrokeOffset
) {
    auto geom = getBoxLikeGeometry(params, width, height);
    double halfW = geom.maskWidth / 2.0;
    double halfH = geom.maskHeight / 2.0;

    if (withStrokeOffset) {
        double offset = getStrokeOffset(params.strokeAlign, params.strokeWidth);
        halfW = std::max(1.0, halfW + offset);
        halfH = std::max(1.0, halfH + offset);
    }

    std::vector<Point2D> points = {
        { geom.centerX, geom.centerY - halfH },
        { geom.centerX + halfW, geom.centerY },
        { geom.centerX, geom.centerY + halfH },
        { geom.centerX - halfW, geom.centerY }
    };

    for (auto& pt : points) {
        pt = rotatePoint(pt.x, pt.y, geom.centerX, geom.centerY, geom.rotationRad);
    }

    return points;
}

HeartCurves BuiltinMaskGeometry::buildHeartCurves(
    const RectangleMaskParams& params,
    double width,
    double height,
    bool withStrokeOffset
) {
    auto geom = getBoxLikeGeometry(params, width, height);
    double halfW = geom.maskWidth / 2.0;
    double halfH = geom.maskHeight / 2.0;

    if (withStrokeOffset) {
        double offset = getStrokeOffset(params.strokeAlign, params.strokeWidth);
        halfW = std::max(1.0, halfW + offset);
        halfH = std::max(1.0, halfH + offset);
    }

    auto toPoint = [&](double localX, double localY) -> Point2D {
        return rotatePoint(
            geom.centerX + localX,
            geom.centerY + localY,
            geom.centerX,
            geom.centerY,
            geom.rotationRad
        );
    };

    Point2D start = toPoint(0.0, -halfH * 0.475);
    Point2D rightCp1 = toPoint(halfW, -halfH * 1.225);
    Point2D rightCp2 = toPoint(halfW, -halfH * 0.125);
    Point2D bottom = toPoint(0.0, halfH * 0.725);
    Point2D leftCp1 = toPoint(-halfW, -halfH * 0.125);
    Point2D leftCp2 = toPoint(-halfW, -halfH * 1.225);

    return HeartCurves{
        .rightBranch = { .start = start, .cp1 = rightCp1, .cp2 = rightCp2, .end = bottom },
        .leftBranch = { .start = bottom, .cp1 = leftCp1, .cp2 = leftCp2, .end = start }
    };
}

std::vector<Point2D> BuiltinMaskGeometry::buildStarVertices(
    const RectangleMaskParams& params,
    double width,
    double height,
    bool withStrokeOffset
) {
    auto geom = getBoxLikeGeometry(params, width, height);
    double halfW = geom.maskWidth / 2.0;
    double halfH = geom.maskHeight / 2.0;

    if (withStrokeOffset) {
        double offset = getStrokeOffset(params.strokeAlign, params.strokeWidth);
        halfW = std::max(1.0, halfW + offset);
        halfH = std::max(1.0, halfH + offset);
    }

    std::vector<Point2D> vertices;
    vertices.reserve(STAR_VERTEX_COUNT);

    for (size_t i = 0; i < STAR_VERTEX_COUNT; ++i) {
        bool isOuter = (i % 2 == 0);
        double radX = isOuter ? halfW : halfW * STAR_INNER_RADIUS_RATIO;
        double radY = isOuter ? halfH : halfH * STAR_INNER_RADIUS_RATIO;
        double angle = (static_cast<double>(i) * M_PI) / 5.0 - M_PI / 2.0;

        Point2D pt = rotatePoint(
            geom.centerX + radX * std::cos(angle),
            geom.centerY + radY * std::sin(angle),
            geom.centerX,
            geom.centerY,
            geom.rotationRad
        );
        vertices.push_back(pt);
    }

    return vertices;
}

SplitLineGeometry BuiltinMaskGeometry::splitLineGeometry(
    double centerX,
    double centerY,
    double rotation,
    double width,
    double height
) noexcept {
    double angleRad = (rotation * M_PI) / 180.0;
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);
    double normalX = (std::abs(c) < NORMAL_SNAP_EPSILON) ? 0.0 : c;
    double normalY = (std::abs(s) < NORMAL_SNAP_EPSILON) ? 0.0 : s;
    double lineX = width / 2.0 + centerX * width;
    double lineY = height / 2.0 + centerY * height;

    return SplitLineGeometry{
        .normalX = normalX,
        .normalY = normalY,
        .lineX = lineX,
        .lineY = lineY
    };
}

double BuiltinMaskGeometry::halfPlaneSign(
    double lineX,
    double lineY,
    double normalX,
    double normalY,
    double x,
    double y
) noexcept {
    return (x - lineX) * normalX + (y - lineY) * normalY;
}

std::optional<Point2D> BuiltinMaskGeometry::lineEdgeIntersection(
    double lineX,
    double lineY,
    double normalX,
    double normalY,
    double x1,
    double y1,
    double x2,
    double y2
) noexcept {
    double d1 = halfPlaneSign(lineX, lineY, normalX, normalY, x1, y1);
    double d2 = halfPlaneSign(lineX, lineY, normalX, normalY, x2, y2);
    double denom = d1 - d2;

    if (std::abs(denom) < 1e-10) {
        return std::nullopt;
    }

    double t = d1 / denom;
    if (t < 0.0 || t > 1.0) {
        return std::nullopt;
    }

    return Point2D{
        .x = x1 + (x2 - x1) * t,
        .y = y1 + (y2 - y1) * t
    };
}

std::optional<std::pair<Point2D, Point2D>> BuiltinMaskGeometry::getSplitMaskStrokeSegment(
    const SplitMaskParams& params,
    double width,
    double height
) {
    auto geom = splitLineGeometry(params.centerX, params.centerY, params.rotation, width, height);

    struct Edge { double x1, y1, x2, y2; };
    Edge edges[4] = {
        { 0.0, 0.0, width, 0.0 },
        { width, 0.0, width, height },
        { width, height, 0.0, height },
        { 0.0, height, 0.0, 0.0 }
    };

    std::vector<Point2D> hits;
    for (const auto& e : edges) {
        auto hit = lineEdgeIntersection(
            geom.lineX, geom.lineY, geom.normalX, geom.normalY,
            e.x1, e.y1, e.x2, e.y2
        );
        if (!hit) {
            continue;
        }

        bool alreadyHas = false;
        for (const auto& pt : hits) {
            if (std::abs(pt.x - hit->x) <= INTERSECTION_EPSILON &&
                std::abs(pt.y - hit->y) <= INTERSECTION_EPSILON) {
                alreadyHas = true;
                break;
            }
        }
        if (!alreadyHas) {
            hits.push_back(*hit);
        }
    }

    if (hits.size() != 2) {
        return std::nullopt;
    }

    return std::make_pair(hits[0], hits[1]);
}

std::vector<Point2D> BuiltinMaskGeometry::buildSplitPolygonVertices(
    const SplitMaskParams& params,
    double width,
    double height
) {
    auto geom = splitLineGeometry(params.centerX, params.centerY, params.rotation, width, height);

    struct Edge { double x1, y1, x2, y2; };
    Edge edges[4] = {
        { 0.0, 0.0, width, 0.0 },
        { width, 0.0, width, height },
        { width, height, 0.0, height },
        { 0.0, height, 0.0, 0.0 }
    };

    auto isInside = [&](double x, double y) {
        return halfPlaneSign(geom.lineX, geom.lineY, geom.normalX, geom.normalY, x, y) >= 0.0;
    };

    std::vector<Point2D> vertices;
    for (const auto& e : edges) {
        bool v1In = isInside(e.x1, e.y1);
        bool v2In = isInside(e.x2, e.y2);

        if (v1In && v2In) {
            vertices.push_back(Point2D{ .x = e.x2, .y = e.y2 });
        } else if (v1In && !v2In) {
            auto hit = lineEdgeIntersection(
                geom.lineX, geom.lineY, geom.normalX, geom.normalY,
                e.x1, e.y1, e.x2, e.y2
            );
            if (hit) {
                vertices.push_back(*hit);
            }
        } else if (!v1In && v2In) {
            auto hit = lineEdgeIntersection(
                geom.lineX, geom.lineY, geom.normalX, geom.normalY,
                e.x1, e.y1, e.x2, e.y2
            );
            if (hit) {
                vertices.push_back(*hit);
                vertices.push_back(Point2D{ .x = e.x2, .y = e.y2 });
            }
        }
    }

    if (vertices.size() < 3 || computePolygonArea(vertices) < MIN_POLYGON_AREA_PX) {
        return {};
    }

    return vertices;
}

double BuiltinMaskGeometry::computePolygonArea(const std::vector<Point2D>& vertices) noexcept {
    double area = 0.0;
    size_t n = vertices.size();
    for (size_t i = 0; i < n; ++i) {
        const auto& p1 = vertices[i];
        const auto& p2 = vertices[(i + 1) % n];
        area += p1.x * p2.y - p2.x * p1.y;
    }
    return std::abs(area) * 0.5;
}

} // namespace catchim::render
