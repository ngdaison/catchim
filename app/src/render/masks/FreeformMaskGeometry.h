#pragma once

#include "render/graphics/GraphicGeometry.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace catchim::render {

struct FreeformPathPoint {
    std::string id;
    double x{0.0};
    double y{0.0};
    double inX{0.0};
    double inY{0.0};
    double outX{0.0};
    double outY{0.0};

    bool operator==(const FreeformPathPoint& other) const noexcept {
        return id == other.id &&
               x == other.x && y == other.y &&
               inX == other.inX && inY == other.inY &&
               outX == other.outX && outY == other.outY;
    }
};

/**
 * @brief Freeform Bezier Path Mask geometry processing and de Casteljau segment subdivision.
 * Corresponds to web/src/masks/freeform/path.ts and web/src/commands/timeline/element/masks/.
 */
class FreeformMaskGeometry {
public:
    static std::vector<FreeformPathPoint> parseFreeformPath(const std::string& jsonString);
    static std::string serializeFreeformPath(const std::vector<FreeformPathPoint>& points);

    static size_t getFreeformSegmentCount(const std::vector<FreeformPathPoint>& points, bool isClosed = true) noexcept;

    static Vec2D evaluateCubicBezier(
        const Vec2D& p0,
        const Vec2D& c0,
        const Vec2D& c1,
        const Vec2D& p1,
        double t
    ) noexcept;

    /**
     * @brief Subdivides a cubic bezier segment using de Casteljau's algorithm and inserts
     * a new point with exact continuous tangent control handles.
     * @return Unique ID of the newly inserted point.
     */
    static std::string insertPointOnSegment(
        std::vector<FreeformPathPoint>& points,
        size_t segmentIndex,
        double t = 0.5,
        bool isClosed = true
    );

    /**
     * @brief Translates all points so that the path bounding center is normalized at (0, 0).
     * @return The centroid offset (cx, cy) that was subtracted.
     */
    static Vec2D recenterPath(std::vector<FreeformPathPoint>& points);

    /**
     * @brief Removes points with the specified IDs from the point list.
     */
    static std::vector<FreeformPathPoint> removeFreeformPathPoints(
        const std::vector<FreeformPathPoint>& points,
        const std::vector<std::string>& pointIds
    );

    /**
     * @brief Evaluates whether a path remains closed after point deletion (needs wasClosed && remaining >= 3).
     */
    static bool getFreeformPathClosedStateAfterPointRemoval(
        bool wasClosed,
        size_t remainingPointCount
    ) noexcept;
};

} // namespace catchim::render
