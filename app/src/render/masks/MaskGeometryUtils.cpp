#include "MaskGeometryUtils.h"

namespace catchim::render {

double MaskGeometryUtils::halfPlaneSign(
    double lineX,
    double lineY,
    double normalX,
    double normalY,
    double x,
    double y
) noexcept {
    return (x - lineX) * normalX + (y - lineY) * normalY;
}

std::optional<MaskPoint2D> MaskGeometryUtils::lineEdgeIntersection(
    double lineX,
    double lineY,
    double normalX,
    double normalY,
    double x1,
    double y1,
    double x2,
    double y2
) noexcept {
    const double distance1 = halfPlaneSign(lineX, lineY, normalX, normalY, x1, y1);
    const double distance2 = halfPlaneSign(lineX, lineY, normalX, normalY, x2, y2);
    const double denom = distance1 - distance2;

    if (std::abs(denom) < LINE_PARALLEL_EPSILON) {
        return std::nullopt;
    }

    const double t = distance1 / denom;
    if (t < 0.0 || t > 1.0) {
        return std::nullopt;
    }

    return MaskPoint2D{
        .x = x1 + (x2 - x1) * t,
        .y = y1 + (y2 - y1) * t
    };
}

double MaskGeometryUtils::computeFeatherUpdate(
    double startFeather,
    double deltaX,
    double deltaY,
    double directionX,
    double directionY
) noexcept {
    const double projection = deltaX * directionX + deltaY * directionY;
    const double updated = std::round(startFeather + projection / FEATHER_HANDLE_SCALE);
    return std::clamp(updated, 0.0, MAX_FEATHER);
}

} // namespace catchim::render
