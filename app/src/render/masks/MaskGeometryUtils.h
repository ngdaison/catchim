#pragma once

#include <optional>
#include <cmath>
#include <algorithm>

namespace catchim::render {

struct MaskPoint2D {
    double x{0.0};
    double y{0.0};

    bool operator==(const MaskPoint2D& other) const noexcept = default;
};

class MaskGeometryUtils {
public:
    static constexpr double MAX_FEATHER = 1000.0;
    static constexpr double FEATHER_HANDLE_SCALE = 0.11;
    static constexpr double DEFAULT_SHAPE_MASK_SHORT_SIDE_RATIO = 0.6;
    static constexpr double MIN_MASK_DIMENSION = 0.01;
    static constexpr double LINE_PARALLEL_EPSILON = 1e-10;

    static double halfPlaneSign(
        double lineX,
        double lineY,
        double normalX,
        double normalY,
        double x,
        double y
    ) noexcept;

    static std::optional<MaskPoint2D> lineEdgeIntersection(
        double lineX,
        double lineY,
        double normalX,
        double normalY,
        double x1,
        double y1,
        double x2,
        double y2
    ) noexcept;

    static double computeFeatherUpdate(
        double startFeather,
        double deltaX,
        double deltaY,
        double directionX,
        double directionY
    ) noexcept;
};

} // namespace catchim::render
