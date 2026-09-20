#pragma once

#include "GraphicGeometry.h"
#include <cmath>
#include <string>
#include <vector>

namespace catchim::render {

struct GraphicPoint {
    double x{0.0};
    double y{0.0};

    bool operator==(const GraphicPoint& other) const noexcept {
        constexpr double EPSILON = 1e-4;
        return std::abs(x - other.x) <= EPSILON && std::abs(y - other.y) <= EPSILON;
    }
};

struct RectangleBounds {
    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};
    double cornerRadius{0.0};
};

struct EllipseBounds {
    double centerX{0.0};
    double centerY{0.0};
    double radiusX{0.0};
    double radiusY{0.0};
};

class GraphicsDefinitions {
public:
    static double calculateStrokeInset(GraphicStrokeAlign align, double strokeWidth) noexcept;

    static RectangleBounds calculateRectangleBounds(
        double width,
        double height,
        double strokeWidth,
        GraphicStrokeAlign align,
        double cornerRadiusPercent) noexcept;

    static EllipseBounds calculateEllipseBounds(
        double width,
        double height,
        double strokeWidth,
        GraphicStrokeAlign align) noexcept;

    static std::vector<GraphicPoint> buildPolygonVertices(
        double centerX,
        double centerY,
        double radius,
        int sides);

    static std::vector<GraphicPoint> buildStarVertices(
        double centerX,
        double centerY,
        int points,
        double outerRadius,
        double innerRadius);

    static std::vector<GraphicPoint> buildStarVerticesWithDepth(
        double centerX,
        double centerY,
        int points,
        double outerRadius,
        double depthPercent);
};

} // namespace catchim::render
