#pragma once

#include <vector>

namespace catchim::render {

enum class GraphicStrokeAlign {
    Center,
    Inside,
    Outside
};

struct Vec2D {
    double x{0.0};
    double y{0.0};

    bool operator==(const Vec2D& other) const noexcept {
        return x == other.x && y == other.y;
    }
};

struct BoundingBox {
    double minX{0.0};
    double minY{0.0};
    double maxX{0.0};
    double maxY{0.0};

    double width() const noexcept { return maxX - minX; }
    double height() const noexcept { return maxY - minY; }
};

/**
 * @brief Parametric Star geometry generator.
 * Corresponds to web/src/graphics/definitions/star.ts.
 */
class StarGeometry {
public:
    static std::vector<Vec2D> generateVertices(
        double width,
        double height,
        int points = 5,
        double depth = 0.45,
        double strokeWidth = 0.0,
        GraphicStrokeAlign strokeAlign = GraphicStrokeAlign::Center
    );
};

/**
 * @brief Parametric regular polygon geometry generator.
 * Corresponds to web/src/graphics/definitions/polygon.ts.
 */
class PolygonGeometry {
public:
    static std::vector<Vec2D> generateVertices(
        double width,
        double height,
        int sides = 6,
        double strokeWidth = 0.0,
        GraphicStrokeAlign strokeAlign = GraphicStrokeAlign::Center
    );
};

/**
 * @brief Aligned stroke metric calculations for Center, Inside, Outside strokes.
 * Corresponds to web/src/graphics/stroke.ts.
 */
class AlignedStrokeMetrics {
public:
    static double getEffectiveStrokeWidth(double strokeWidth, GraphicStrokeAlign strokeAlign) noexcept;
    static BoundingBox computeExpandedBounds(
        double width,
        double height,
        double strokeWidth,
        GraphicStrokeAlign strokeAlign
    ) noexcept;
};

} // namespace catchim::render
