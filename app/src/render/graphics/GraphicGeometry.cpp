#include "GraphicGeometry.h"
#include <cmath>
#include <algorithm>
#include <numbers>

namespace catchim::render {

std::vector<Vec2D> StarGeometry::generateVertices(
    double width,
    double height,
    int points,
    double depth,
    double strokeWidth,
    GraphicStrokeAlign strokeAlign
) {
    int clampedPoints = std::clamp(points, 3, 12);
    double clampedDepth = std::clamp(depth, 0.01, 0.99);
    double inset = (strokeAlign == GraphicStrokeAlign::Center) ? (std::max(0.0, strokeWidth) / 2.0) : 0.0;

    double minDim = std::min(width, height);
    double outerRadius = std::max(1.0, (minDim / 2.0) - inset);
    double innerRadius = outerRadius * clampedDepth;
    double centerX = width / 2.0;
    double centerY = height / 2.0;

    int totalVertices = clampedPoints * 2;
    std::vector<Vec2D> vertices;
    vertices.reserve(static_cast<size_t>(totalVertices));

    const double pi = std::numbers::pi;
    for (int i = 0; i < totalVertices; ++i) {
        double radius = (i % 2 == 0) ? outerRadius : innerRadius;
        double angle = -pi / 2.0 + (static_cast<double>(i) * pi) / static_cast<double>(clampedPoints);
        double x = centerX + std::cos(angle) * radius;
        double y = centerY + std::sin(angle) * radius;
        vertices.push_back(Vec2D{x, y});
    }

    return vertices;
}

std::vector<Vec2D> PolygonGeometry::generateVertices(
    double width,
    double height,
    int sides,
    double strokeWidth,
    GraphicStrokeAlign strokeAlign
) {
    int clampedSides = std::clamp(sides, 3, 12);
    double inset = (strokeAlign == GraphicStrokeAlign::Center) ? (std::max(0.0, strokeWidth) / 2.0) : 0.0;

    double minDim = std::min(width, height);
    double radius = std::max(1.0, (minDim / 2.0) - inset);
    double centerX = width / 2.0;
    double centerY = height / 2.0;

    std::vector<Vec2D> vertices;
    vertices.reserve(static_cast<size_t>(clampedSides));

    const double pi = std::numbers::pi;
    for (int i = 0; i < clampedSides; ++i) {
        double angle = -pi / 2.0 + (static_cast<double>(i) * 2.0 * pi) / static_cast<double>(clampedSides);
        double x = centerX + std::cos(angle) * radius;
        double y = centerY + std::sin(angle) * radius;
        vertices.push_back(Vec2D{x, y});
    }

    return vertices;
}

double AlignedStrokeMetrics::getEffectiveStrokeWidth(
    double strokeWidth,
    GraphicStrokeAlign strokeAlign
) noexcept {
    if (strokeWidth <= 0.0) return 0.0;
    switch (strokeAlign) {
        case GraphicStrokeAlign::Center:
            return strokeWidth;
        case GraphicStrokeAlign::Inside:
        case GraphicStrokeAlign::Outside:
            return strokeWidth * 2.0;
    }
    return strokeWidth;
}

BoundingBox AlignedStrokeMetrics::computeExpandedBounds(
    double width,
    double height,
    double strokeWidth,
    GraphicStrokeAlign strokeAlign
) noexcept {
    double expansion = 0.0;
    if (strokeWidth > 0.0) {
        if (strokeAlign == GraphicStrokeAlign::Center) {
            expansion = strokeWidth / 2.0;
        } else if (strokeAlign == GraphicStrokeAlign::Outside) {
            expansion = strokeWidth;
        }
        // For Inside, expansion is 0.0 (stroke is contained strictly inside the path)
    }

    return BoundingBox{
        -expansion,
        -expansion,
        width + expansion,
        height + expansion
    };
}

} // namespace catchim::render
