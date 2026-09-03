#pragma once

#include <cmath>
#include <vector>
#include "opencut/compositor.hpp"
#include "opencut/scene.hpp"

namespace opencut::geometry {

class GeometryEngine {
public:
    // Checks if a 2D point lies within an oriented (rotated, centered) rectangle
    static bool point_in_rotated_rect(
        double px, double py,
        double center_x, double center_y,
        double width, double height,
        double rotation_degrees
    );

    // Checks if an arbitrary transformed 4-vertex quad intersects an axis-aligned box (SAT test)
    static bool quad_intersects_rect(
        const Point2D quad[4],
        const scene::Rect2D& rect
    );

    // Calculates distance and snapped delta coordinate for alignment guides
    static bool test_snap_axis(
        double source_val,
        double target_val,
        double threshold,
        double* out_snapped_val,
        double* out_delta
    );
};

} // namespace opencut::geometry
