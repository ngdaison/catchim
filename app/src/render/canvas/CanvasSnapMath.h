#pragma once

#include <cmath>
#include <algorithm>

namespace catchim::render {

struct CanvasSnapResult {
    double x{0.0};
    double y{0.0};
    bool snappedX{false};
    bool snappedY{false};
};

class CanvasSnapMath {
public:
    static constexpr double DEFAULT_TOLERANCE = 8.0;
    static constexpr double DEFAULT_ANGLE_TOLERANCE_DEG = 5.0;

    static double snapAngle(
        double angleDegrees,
        double toleranceDegrees = DEFAULT_ANGLE_TOLERANCE_DEG
    ) noexcept;

    static double snapScalar(
        double value,
        double target,
        double tolerance = DEFAULT_TOLERANCE
    ) noexcept;

    static CanvasSnapResult snapToCanvas(
        double x,
        double y,
        double width,
        double height,
        double canvasWidth,
        double canvasHeight,
        double tolerance = DEFAULT_TOLERANCE
    ) noexcept;
};

} // namespace catchim::render
