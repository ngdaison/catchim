#include "opencut/masks.hpp"

#include <algorithm>
#include <cmath>

namespace opencut {

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float DEG_TO_RAD = PI / 180.0f;

float length(Point2D v) noexcept {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Point2D rotate_point(Point2D p, float degrees) noexcept {
    float rad = -degrees * DEG_TO_RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    return {p.x * c - p.y * s, p.x * s + p.y * c};
}

float smoothstep(float edge0, float edge1, float x) noexcept {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

float sdf_rectangle(Point2D p, Point2D half_size) noexcept {
    float dx = std::abs(p.x) - half_size.x;
    float dy = std::abs(p.y) - half_size.y;
    float outside_dist = length({std::max(dx, 0.0f), std::max(dy, 0.0f)});
    float inside_dist = std::min(std::max(dx, dy), 0.0f);
    return outside_dist + inside_dist;
}

float sdf_circle(Point2D p, float radius) noexcept {
    return length(p) - radius;
}

float sdf_ellipse(Point2D p, Point2D radii) noexcept {
    if (radii.x <= 0.0f || radii.y <= 0.0f) return 0.0f;
    float k0 = length({p.x / radii.x, p.y / radii.y});
    float k1 = length({p.x / (radii.x * radii.x), p.y / (radii.y * radii.y)});
    return k1 > 0.0f ? (k0 * (k0 - 1.0f) / k1) : 0.0f;
}

float sdf_linear(Point2D p, Point2D normal) noexcept {
    float len = length(normal);
    if (len <= 0.0f) return 0.0f;
    return (p.x * normal.x + p.y * normal.y) / len;
}

float sdf_polygon(Point2D p, std::span<const Point2D> vertices) noexcept {
    if (vertices.size() < 3) return 1.0f;
    float min_dist_sq = 1e30f;
    bool inside = false;

    for (std::size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
        Point2D vi = vertices[i];
        Point2D vj = vertices[j];

        // Ray casting for inside/outside test
        if (((vi.y > p.y) != (vj.y > p.y)) &&
            (p.x < (vj.x - vi.x) * (p.y - vi.y) / (vj.y - vi.y + 1e-7f) + vi.x)) {
            inside = !inside;
        }

        // Distance to line segment
        Point2D seg = {vj.x - vi.x, vj.y - vi.y};
        Point2D to_p = {p.x - vi.x, p.y - vi.y};
        float seg_len_sq = seg.x * seg.x + seg.y * seg.y;
        float t = seg_len_sq > 0.0f ? std::clamp((to_p.x * seg.x + to_p.y * seg.y) / seg_len_sq, 0.0f, 1.0f) : 0.0f;
        Point2D closest = {vi.x + t * seg.x, vi.y + t * seg.y};
        float dist_sq = (p.x - closest.x) * (p.x - closest.x) + (p.y - closest.y) * (p.y - closest.y);
        min_dist_sq = std::min(min_dist_sq, dist_sq);
    }

    float dist = std::sqrt(min_dist_sq);
    return inside ? -dist : dist;
}

float apply_feather(float distance, float feather_radius, bool inverted) noexcept {
    float alpha = 0.0f;
    if (feather_radius <= 0.001f) {
        alpha = distance <= 0.0f ? 1.0f : 0.0f;
    } else {
        float half = feather_radius * 0.5f;
        alpha = 1.0f - smoothstep(-half, half, distance);
    }
    return inverted ? (1.0f - alpha) : alpha;
}

float evaluate_mask_alpha(Point2D p, const MaskDefinition& mask) noexcept {
    Point2D local = {p.x - mask.center.x, p.y - mask.center.y};
    local = rotate_point(local, mask.rotation_degrees);

    float d = 0.0f;
    switch (mask.type) {
        case MaskShapeType::Rectangle:
            d = sdf_rectangle(local, {mask.size.x * 0.5f, mask.size.y * 0.5f});
            break;
        case MaskShapeType::Radial:
            d = sdf_ellipse(local, {mask.size.x * 0.5f, mask.size.y * 0.5f});
            break;
        case MaskShapeType::Linear:
            d = sdf_linear(local, {0.0f, 1.0f});
            break;
        case MaskShapeType::Mirror: {
            float half_gap = mask.size.y * 0.5f;
            d = std::abs(local.y) - half_gap;
            break;
        }
        case MaskShapeType::Polygon:
            d = sdf_polygon(local, mask.polygon_points);
            break;
    }

    return apply_feather(d, mask.feather, mask.inverted);
}

} // namespace opencut
