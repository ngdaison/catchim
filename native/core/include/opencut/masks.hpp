#pragma once

#include "opencut/compositor.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace opencut {

enum class MaskShapeType {
    Linear,
    Radial,
    Rectangle,
    Mirror,
    Polygon,
};

struct MaskDefinition {
    MaskShapeType type = MaskShapeType::Rectangle;
    Point2D center = {0.0f, 0.0f};
    Point2D size = {1.0f, 1.0f};
    float rotation_degrees = 0.0f;
    float feather = 0.0f;
    bool inverted = false;
    std::vector<Point2D> polygon_points;
};

// Analytical Signed Distance Field (SDF) functions
[[nodiscard]] float sdf_rectangle(Point2D p, Point2D half_size) noexcept;
[[nodiscard]] float sdf_circle(Point2D p, float radius) noexcept;
[[nodiscard]] float sdf_ellipse(Point2D p, Point2D radii) noexcept;
[[nodiscard]] float sdf_linear(Point2D p, Point2D normal) noexcept;
[[nodiscard]] float sdf_polygon(Point2D p, std::span<const Point2D> vertices) noexcept;

// Evaluate mask alpha (0.0 - 1.0) at point p
[[nodiscard]] float evaluate_mask_alpha(Point2D p, const MaskDefinition& mask) noexcept;

// Soft feathering curve given signed distance and feather width
[[nodiscard]] float apply_feather(float distance, float feather_radius, bool inverted) noexcept;

} // namespace opencut
