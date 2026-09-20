#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cmath>

namespace catchim::render {

struct Point2D {
    double x{0.0};
    double y{0.0};

    bool operator==(const Point2D& other) const = default;
};

struct Rect2D {
    double x{0.0};
    double y{0.0};
    double w{0.0};
    double h{0.0};

    [[nodiscard]] double left() const noexcept { return x; }
    [[nodiscard]] double right() const noexcept { return x + w; }
    [[nodiscard]] double top() const noexcept { return y; }
    [[nodiscard]] double bottom() const noexcept { return y + h; }
};

enum class GizmoHandle {
    None,
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left,
    Rotate
};

class HitTesting {
public:
    static bool containsPoint(
        const Rect2D& rect,
        const Point2D& pt
    ) noexcept;

    static bool containsPointRotated(
        const Rect2D& rect,
        double rotationDeg,
        const Point2D& anchor,
        const Point2D& pt
    ) noexcept;

    static GizmoHandle hitTestGizmoHandle(
        const Rect2D& rect,
        double rotationDeg,
        const Point2D& pt,
        double handleRadius = 8.0
    ) noexcept;

    static bool intersects(
        const Rect2D& r1,
        const Rect2D& r2
    ) noexcept;

    static std::vector<std::string> resolveElementIntersections(
        const Rect2D& selectionRect,
        const std::vector<std::pair<std::string, Rect2D>>& elements
    );
};

} // namespace catchim::render
