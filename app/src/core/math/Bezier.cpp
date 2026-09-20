#include "Bezier.h"
#include <cmath>
#include <algorithm>

namespace catchim::core {

namespace {
constexpr double EPSILON = 1e-6;
constexpr int MAX_ITERATIONS = 16;

inline double sampleCurve(double t, double p1, double p2) {
    // p0 = 0, p3 = 1
    // B(t) = 3*(1-t)^2*t*p1 + 3*(1-t)*t^2*p2 + t^3
    double oneMinusT = 1.0 - t;
    return 3.0 * oneMinusT * oneMinusT * t * p1 +
           3.0 * oneMinusT * t * t * p2 +
           t * t * t;
}

inline double sampleCurveDerivative(double t, double p1, double p2) {
    double oneMinusT = 1.0 - t;
    return 3.0 * oneMinusT * oneMinusT * p1 +
           6.0 * oneMinusT * t * (p2 - p1) +
           3.0 * t * t * (1.0 - p2);
}
} // namespace

double BezierSolver::evaluatePoint(double t, double p0, double p1, double p2, double p3) {
    double oneMinusT = 1.0 - t;
    return oneMinusT * oneMinusT * oneMinusT * p0 +
           3.0 * oneMinusT * oneMinusT * t * p1 +
           3.0 * oneMinusT * t * t * p2 +
           t * t * t * p3;
}

double BezierSolver::solve(double time, double x1, double y1, double x2, double y2) {
    time = std::clamp(time, 0.0, 1.0);
    if (time <= 0.0) return 0.0;
    if (time >= 1.0) return 1.0;

    // Newton-Raphson method
    double t = time;
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        double currentX = sampleCurve(t, x1, x2) - time;
        if (std::abs(currentX) < EPSILON) break;

        double dX = sampleCurveDerivative(t, x1, x2);
        if (std::abs(dX) < 1e-8) break;

        t -= currentX / dX;
    }

    t = std::clamp(t, 0.0, 1.0);
    return sampleCurve(t, y1, y2);
}

} // namespace catchim::core
