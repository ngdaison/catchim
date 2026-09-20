#include "FreeformMaskGeometry.h"
#include "core/ids/Ids.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>

namespace catchim::render {

namespace {
inline Vec2D lerp2D(const Vec2D& a, const Vec2D& b, double t) {
    return Vec2D{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    };
}
} // namespace

std::vector<FreeformPathPoint> FreeformMaskGeometry::parseFreeformPath(const std::string& jsonString) {
    std::vector<FreeformPathPoint> result;
    if (jsonString.empty()) return result;

    try {
        auto parsed = nlohmann::json::parse(jsonString);
        if (parsed.is_array()) {
            for (const auto& item : parsed) {
                if (item.is_object() && item.contains("id") && item.contains("x") && item.contains("y")) {
                    FreeformPathPoint pt;
                    pt.id = item["id"].get<std::string>();
                    pt.x = item["x"].get<double>();
                    pt.y = item["y"].get<double>();
                    pt.inX = item.value("inX", 0.0);
                    pt.inY = item.value("inY", 0.0);
                    pt.outX = item.value("outX", 0.0);
                    pt.outY = item.value("outY", 0.0);
                    result.push_back(std::move(pt));
                }
            }
        }
    } catch (...) {}

    return result;
}

std::string FreeformMaskGeometry::serializeFreeformPath(const std::vector<FreeformPathPoint>& points) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& pt : points) {
        nlohmann::json item;
        item["id"] = pt.id;
        item["x"] = pt.x;
        item["y"] = pt.y;
        item["inX"] = pt.inX;
        item["inY"] = pt.inY;
        item["outX"] = pt.outX;
        item["outY"] = pt.outY;
        arr.push_back(std::move(item));
    }
    return arr.dump();
}

size_t FreeformMaskGeometry::getFreeformSegmentCount(
    const std::vector<FreeformPathPoint>& points,
    bool isClosed
) noexcept {
    if (points.size() < 2) return 0;
    return isClosed ? points.size() : points.size() - 1;
}

Vec2D FreeformMaskGeometry::evaluateCubicBezier(
    const Vec2D& p0,
    const Vec2D& c0,
    const Vec2D& c1,
    const Vec2D& p1,
    double t
) noexcept {
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double uuu = uu * u;
    double ttt = tt * t;

    double x = uuu * p0.x + 3.0 * uu * t * c0.x + 3.0 * u * tt * c1.x + ttt * p1.x;
    double y = uuu * p0.y + 3.0 * uu * t * c0.y + 3.0 * u * tt * c1.y + ttt * p1.y;
    return Vec2D{x, y};
}

std::string FreeformMaskGeometry::insertPointOnSegment(
    std::vector<FreeformPathPoint>& points,
    size_t segmentIndex,
    double t,
    bool isClosed
) {
    size_t segCount = getFreeformSegmentCount(points, isClosed);
    if (segCount == 0 || segmentIndex >= segCount) {
        return "";
    }

    size_t idx0 = segmentIndex;
    size_t idx1 = (segmentIndex + 1) % points.size();

    auto p0 = points[idx0];
    auto p1 = points[idx1];

    Vec2D P0{p0.x, p0.y};
    Vec2D C0{p0.x + p0.outX, p0.y + p0.outY};
    Vec2D C1{p1.x + p1.inX, p1.y + p1.inY};
    Vec2D P1{p1.x, p1.y};

    // de Casteljau subdivision
    Vec2D A = lerp2D(P0, C0, t);
    Vec2D B = lerp2D(C0, C1, t);
    Vec2D C = lerp2D(C1, P1, t);

    Vec2D D = lerp2D(A, B, t);
    Vec2D E = lerp2D(B, C, t);

    Vec2D F = lerp2D(D, E, t); // Split point anchor

    // Update p0 out-handle and p1 in-handle
    p0.outX = A.x - P0.x;
    p0.outY = A.y - P0.y;

    p1.inX = C.x - P1.x;
    p1.inY = C.y - P1.y;

    points[idx0] = p0;
    points[idx1] = p1;

    // Create inserted point
    FreeformPathPoint newPt;
    newPt.id = core::detail::generateRandomId(12);
    newPt.x = F.x;
    newPt.y = F.y;
    newPt.inX = D.x - F.x;
    newPt.inY = D.y - F.y;
    newPt.outX = E.x - F.x;
    newPt.outY = E.y - F.y;

    std::string newId = newPt.id;
    points.insert(points.begin() + static_cast<std::ptrdiff_t>(segmentIndex + 1), std::move(newPt));

    return newId;
}

Vec2D FreeformMaskGeometry::recenterPath(std::vector<FreeformPathPoint>& points) {
    if (points.empty()) {
        return Vec2D{0.0, 0.0};
    }

    double minX = points.front().x;
    double maxX = points.front().x;
    double minY = points.front().y;
    double maxY = points.front().y;

    for (const auto& pt : points) {
        minX = std::min(minX, pt.x);
        maxX = std::max(maxX, pt.x);
        minY = std::min(minY, pt.y);
        maxY = std::max(maxY, pt.y);
    }

    double cx = (minX + maxX) / 2.0;
    double cy = (minY + maxY) / 2.0;

    for (auto& pt : points) {
        pt.x -= cx;
        pt.y -= cy;
    }

    return Vec2D{cx, cy};
}

std::vector<FreeformPathPoint> FreeformMaskGeometry::removeFreeformPathPoints(
    const std::vector<FreeformPathPoint>& points,
    const std::vector<std::string>& pointIds
) {
    if (pointIds.empty()) {
        return points;
    }
    std::unordered_set<std::string> toRemove(pointIds.begin(), pointIds.end());
    std::vector<FreeformPathPoint> remaining;
    remaining.reserve(points.size());
    for (const auto& pt : points) {
        if (!toRemove.contains(pt.id)) {
            remaining.push_back(pt);
        }
    }
    return remaining;
}

bool FreeformMaskGeometry::getFreeformPathClosedStateAfterPointRemoval(
    bool wasClosed,
    size_t remainingPointCount
) noexcept {
    return wasClosed && remainingPointCount >= 3;
}

} // namespace catchim::render
