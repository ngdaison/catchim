#pragma once

namespace catchim::core {

struct CubicBezier {
    double x1{0.25};
    double y1{0.1};
    double x2{0.25};
    double y2{1.0};

    static constexpr CubicBezier linear() noexcept { return {0.0, 0.0, 1.0, 1.0}; }
    static constexpr CubicBezier ease() noexcept { return {0.25, 0.1, 0.25, 1.0}; }
    static constexpr CubicBezier easeIn() noexcept { return {0.42, 0.0, 1.0, 1.0}; }
    static constexpr CubicBezier easeOut() noexcept { return {0.0, 0.0, 0.58, 1.0}; }
    static constexpr CubicBezier easeInOut() noexcept { return {0.42, 0.0, 0.58, 1.0}; }
};

class BezierSolver {
public:
    static double solve(double time, double x1, double y1, double x2, double y2);
    static double evaluatePoint(double t, double p0, double p1, double p2, double p3);
};

} // namespace catchim::core
