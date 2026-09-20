#include "GraphicsDefinitions.h"

#include <algorithm>
#include <numbers>

namespace catchim::render {

double GraphicsDefinitions::calculateStrokeInset(
    GraphicStrokeAlign align,
    double strokeWidth) noexcept {

    const double width = std::max(0.0, strokeWidth);
    switch (align) {
    case GraphicStrokeAlign::Center:
        return width / 2.0;
    case GraphicStrokeAlign::Inside:
        return width;
    case GraphicStrokeAlign::Outside:
        return 0.0;
    }
    return width / 2.0;
}

RectangleBounds GraphicsDefinitions::calculateRectangleBounds(
    double width,
    double height,
    double strokeWidth,
    GraphicStrokeAlign align,
    double cornerRadiusPercent) noexcept {

    const double inset = calculateStrokeInset(align, strokeWidth);
    const double drawWidth = std::max(1.0, width - inset * 2.0);
    const double drawHeight = std::max(1.0, height - inset * 2.0);
    const double clampedPercent = std::max(0.0, std::min(50.0, cornerRadiusPercent));
    const double radius = (std::min(drawWidth, drawHeight) / 2.0) * (clampedPercent / 50.0);

    return RectangleBounds{
        inset,
        inset,
        drawWidth,
        drawHeight,
        radius
    };
}

EllipseBounds GraphicsDefinitions::calculateEllipseBounds(
    double width,
    double height,
    double strokeWidth,
    GraphicStrokeAlign align) noexcept {

    const double inset = calculateStrokeInset(align, strokeWidth);
    const double centerX = width / 2.0;
    const double centerY = height / 2.0;
    const double radiusX = std::max(1.0, width / 2.0 - inset);
    const double radiusY = std::max(1.0, height / 2.0 - inset);

    return EllipseBounds{
        centerX,
        centerY,
        radiusX,
        radiusY
    };
}

std::vector<GraphicPoint> GraphicsDefinitions::buildPolygonVertices(
    double centerX,
    double centerY,
    double radius,
    int sides) {

    const int clampedSides = std::max(3, std::min(64, sides));
    std::vector<GraphicPoint> points;
    points.reserve(clampedSides);

    constexpr double PI = 3.14159265358979323846;
    for (int i = 0; i < clampedSides; ++i) {
        const double angle = -PI / 2.0 + (static_cast<double>(i) * PI * 2.0) / static_cast<double>(clampedSides);
        points.push_back(GraphicPoint{
            centerX + std::cos(angle) * radius,
            centerY + std::sin(angle) * radius
        });
    }

    return points;
}

std::vector<GraphicPoint> GraphicsDefinitions::buildStarVertices(
    double centerX,
    double centerY,
    int points,
    double outerRadius,
    double innerRadius) {

    const int clampedPoints = std::max(3, std::min(64, points));
    const int totalVertices = clampedPoints * 2;
    std::vector<GraphicPoint> vertices;
    vertices.reserve(totalVertices);

    constexpr double PI = 3.14159265358979323846;
    for (int i = 0; i < totalVertices; ++i) {
        const double radius = (i % 2 == 0) ? outerRadius : innerRadius;
        const double angle = -PI / 2.0 + (static_cast<double>(i) * PI) / static_cast<double>(clampedPoints);
        vertices.push_back(GraphicPoint{
            centerX + std::cos(angle) * radius,
            centerY + std::sin(angle) * radius
        });
    }

    return vertices;
}

std::vector<GraphicPoint> GraphicsDefinitions::buildStarVerticesWithDepth(
    double centerX,
    double centerY,
    int points,
    double outerRadius,
    double depthPercent) {

    const double depth = std::max(0.01, std::min(0.99, depthPercent / 100.0));
    const double innerRadius = outerRadius * depth;
    return buildStarVertices(centerX, centerY, points, outerRadius, innerRadius);
}

} // namespace catchim::render
