#include "opencut/geometry.hpp"
#include <algorithm>
#include <limits>

namespace opencut::geometry {

static constexpr double PI = 3.14159265358979323846;

bool GeometryEngine::point_in_rotated_rect(
    double px, double py,
    double center_x, double center_y,
    double width, double height,
    double rotation_degrees
) {
    if (width <= 0.0 || height <= 0.0) {
        return false;
    }

    const double dx = px - center_x;
    const double dy = py - center_y;

    const double rad = -rotation_degrees * (PI / 180.0);
    const double cos_a = std::cos(rad);
    const double sin_a = std::sin(rad);

    const double local_x = dx * cos_a - dy * sin_a;
    const double local_y = dx * sin_a + dy * cos_a;

    return (std::abs(local_x) <= width * 0.5) && (std::abs(local_y) <= height * 0.5);
}

bool GeometryEngine::test_snap_axis(
    double source_val,
    double target_val,
    double threshold,
    double* out_snapped_val,
    double* out_delta
) {
    const double delta = target_val - source_val;
    if (std::abs(delta) <= threshold) {
        if (out_snapped_val != nullptr) *out_snapped_val = target_val;
        if (out_delta != nullptr) *out_delta = delta;
        return true;
    }
    return false;
}

bool GeometryEngine::quad_intersects_rect(
    const Point2D quad[4],
    const scene::Rect2D& rect
) {
    if (rect.width <= 0.0 || rect.height <= 0.0) {
        return false;
    }

    // 4 vertices of the axis-aligned rect
    const Point2D box[4] = {
        {static_cast<float>(rect.x), static_cast<float>(rect.y)},
        {static_cast<float>(rect.x + rect.width), static_cast<float>(rect.y)},
        {static_cast<float>(rect.x + rect.width), static_cast<float>(rect.y + rect.height)},
        {static_cast<float>(rect.x), static_cast<float>(rect.y + rect.height)}
    };

    // Separating Axis Theorem axes to test:
    // 2 from rect: (1, 0), (0, 1)
    // 2 from quad edges
    std::vector<Point2D> axes;
    axes.push_back({1.0f, 0.0f});
    axes.push_back({0.0f, 1.0f});

    for (int i = 0; i < 2; ++i) {
        float edge_x = quad[(i + 1) % 4].x - quad[i].x;
        float edge_y = quad[(i + 1) % 4].y - quad[i].y;
        float len = std::hypot(edge_x, edge_y);
        if (len > 0.0001f) {
            axes.push_back({-edge_y / len, edge_x / len});
        }
    }

    for (const auto& axis : axes) {
        // Project box
        float min_box = std::numeric_limits<float>::infinity();
        float max_box = -std::numeric_limits<float>::infinity();
        for (const auto& pt : box) {
            float proj = pt.x * axis.x + pt.y * axis.y;
            min_box = std::min(min_box, proj);
            max_box = std::max(max_box, proj);
        }

        // Project quad
        float min_quad = std::numeric_limits<float>::infinity();
        float max_quad = -std::numeric_limits<float>::infinity();
        for (int i = 0; i < 4; ++i) {
            float proj = quad[i].x * axis.x + quad[i].y * axis.y;
            min_quad = std::min(min_quad, proj);
            max_quad = std::max(max_quad, proj);
        }

        // Test for separation
        if (max_box < min_quad || max_quad < min_box) {
            return false; // Found separating axis!
        }
    }

    return true;
}

} // namespace opencut::geometry
