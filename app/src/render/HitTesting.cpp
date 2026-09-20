#include "render/HitTesting.h"
#include <algorithm>

namespace catchim::render {

bool HitTesting::containsPoint(const Rect2D& rect, const Point2D& pt) noexcept {
    return pt.x >= rect.left() && pt.x <= rect.right() &&
           pt.y >= rect.top() && pt.y <= rect.bottom();
}

bool HitTesting::containsPointRotated(
    const Rect2D& rect,
    double rotationDeg,
    const Point2D& anchor,
    const Point2D& pt
) noexcept {
    if (std::abs(rotationDeg) < 1e-4) {
        return containsPoint(rect, pt);
    }

    double rad = -rotationDeg * 3.14159265358979323846 / 180.0;
    double cosA = std::cos(rad);
    double sinA = std::sin(rad);

    double dx = pt.x - anchor.x;
    double dy = pt.y - anchor.y;

    Point2D unrot{
        anchor.x + dx * cosA - dy * sinA,
        anchor.y + dx * sinA + dy * cosA
    };

    return containsPoint(rect, unrot);
}

GizmoHandle HitTesting::hitTestGizmoHandle(
    const Rect2D& rect,
    double rotationDeg,
    const Point2D& pt,
    double handleRadius
) noexcept {
    Point2D anchor{rect.x + rect.w * 0.5, rect.y + rect.h * 0.5};
    Point2D localPt = pt;

    if (std::abs(rotationDeg) >= 1e-4) {
        double rad = -rotationDeg * 3.14159265358979323846 / 180.0;
        double cosA = std::cos(rad);
        double sinA = std::sin(rad);

        double dx = pt.x - anchor.x;
        double dy = pt.y - anchor.y;

        localPt.x = anchor.x + dx * cosA - dy * sinA;
        localPt.y = anchor.y + dx * sinA + dy * cosA;
    }

    double hr2 = handleRadius * handleRadius;
    auto distSq = [](const Point2D& a, const Point2D& b) {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        return dx * dx + dy * dy;
    };

    // Rotate handle: 24px above top center
    Point2D rotatePos{rect.x + rect.w * 0.5, rect.y - 24.0};
    if (distSq(localPt, rotatePos) <= hr2) return GizmoHandle::Rotate;

    // 8 bounding handles
    Point2D tl{rect.left(), rect.top()};
    if (distSq(localPt, tl) <= hr2) return GizmoHandle::TopLeft;

    Point2D tr{rect.right(), rect.top()};
    if (distSq(localPt, tr) <= hr2) return GizmoHandle::TopRight;

    Point2D br{rect.right(), rect.bottom()};
    if (distSq(localPt, br) <= hr2) return GizmoHandle::BottomRight;

    Point2D bl{rect.left(), rect.bottom()};
    if (distSq(localPt, bl) <= hr2) return GizmoHandle::BottomLeft;

    Point2D t{rect.x + rect.w * 0.5, rect.top()};
    if (distSq(localPt, t) <= hr2) return GizmoHandle::Top;

    Point2D r{rect.right(), rect.y + rect.h * 0.5};
    if (distSq(localPt, r) <= hr2) return GizmoHandle::Right;

    Point2D b{rect.x + rect.w * 0.5, rect.bottom()};
    if (distSq(localPt, b) <= hr2) return GizmoHandle::Bottom;

    Point2D l{rect.left(), rect.y + rect.h * 0.5};
    if (distSq(localPt, l) <= hr2) return GizmoHandle::Left;

    return GizmoHandle::None;
}

bool HitTesting::intersects(const Rect2D& r1, const Rect2D& r2) noexcept {
    return !(r1.right() < r2.left() ||
             r1.left() > r2.right() ||
             r1.bottom() < r2.top() ||
             r1.top() > r2.bottom());
}

std::vector<std::string> HitTesting::resolveElementIntersections(
    const Rect2D& selectionRect,
    const std::vector<std::pair<std::string, Rect2D>>& elements
) {
    std::vector<std::string> result;
    for (const auto& [id, rect] : elements) {
        if (intersects(selectionRect, rect)) {
            result.push_back(id);
        }
    }
    return result;
}

} // namespace catchim::render
