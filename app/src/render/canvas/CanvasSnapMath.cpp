#include "CanvasSnapMath.h"

namespace catchim::render {

double CanvasSnapMath::snapAngle(double angleDegrees, double toleranceDegrees) noexcept {
    // Normalize to [0, 360)
    double norm = std::fmod(angleDegrees, 360.0);
    if (norm < 0.0) {
        norm += 360.0;
    }

    // Nearest 90-degree increment
    double nearest = std::round(norm / 90.0) * 90.0;
    if (std::abs(norm - nearest) <= toleranceDegrees) {
        // Map back to original interval
        double diff = nearest - norm;
        return angleDegrees + diff;
    }

    // Also check 360 boundary for 0
    if (std::abs(norm - 360.0) <= toleranceDegrees) {
        double diff = 360.0 - norm;
        return angleDegrees + diff;
    }

    return angleDegrees;
}

double CanvasSnapMath::snapScalar(double value, double target, double tolerance) noexcept {
    if (std::abs(value - target) <= tolerance) {
        return target;
    }
    return value;
}

CanvasSnapResult CanvasSnapMath::snapToCanvas(
    double x,
    double y,
    double width,
    double height,
    double canvasWidth,
    double canvasHeight,
    double tolerance
) noexcept {
    CanvasSnapResult res{
        .x = x,
        .y = y,
        .snappedX = false,
        .snappedY = false
    };

    // Horizontal snapping: center, left edge, right edge
    const double elemCenterX = x + width / 2.0;
    const double canvasCenterX = canvasWidth / 2.0;

    // Center X
    if (std::abs(elemCenterX - canvasCenterX) <= tolerance) {
        res.x = canvasCenterX - width / 2.0;
        res.snappedX = true;
    } else if (std::abs(x) <= tolerance) { // Left edge
        res.x = 0.0;
        res.snappedX = true;
    } else if (std::abs((x + width) - canvasWidth) <= tolerance) { // Right edge
        res.x = canvasWidth - width;
        res.snappedX = true;
    }

    // Vertical snapping: center, top edge, bottom edge
    const double elemCenterY = y + height / 2.0;
    const double canvasCenterY = canvasHeight / 2.0;

    // Center Y
    if (std::abs(elemCenterY - canvasCenterY) <= tolerance) {
        res.y = canvasCenterY - height / 2.0;
        res.snappedY = true;
    } else if (std::abs(y) <= tolerance) { // Top edge
        res.y = 0.0;
        res.snappedY = true;
    } else if (std::abs((y + height) - canvasHeight) <= tolerance) { // Bottom edge
        res.y = canvasHeight - height;
        res.snappedY = true;
    }

    return res;
}

} // namespace catchim::render
