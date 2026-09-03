#include "opencut/speed.hpp"
#include <algorithm>
#include <cmath>

namespace opencut::speed {

SpeedCurve::SpeedCurve(std::vector<SpeedPoint> points) : points_(std::move(points)) {
    sort_and_clean();
}

void SpeedCurve::add_point(double timeline_ratio, double speed_multiplier) {
    points_.push_back(SpeedPoint{
        std::clamp(timeline_ratio, 0.0, 1.0),
        std::max(0.0, speed_multiplier)
    });
    sort_and_clean();
}

void SpeedCurve::clear() {
    points_.clear();
}

void SpeedCurve::sort_and_clean() {
    std::sort(points_.begin(), points_.end(), [](const SpeedPoint& a, const SpeedPoint& b) {
        return a.timeline_ratio < b.timeline_ratio;
    });

    if (!points_.empty() && points_.front().timeline_ratio > 0.0) {
        points_.insert(points_.begin(), SpeedPoint{0.0, points_.front().speed_multiplier});
    }

    if (!points_.empty() && points_.back().timeline_ratio < 1.0) {
        points_.push_back(SpeedPoint{1.0, points_.back().speed_multiplier});
    }
}

double SpeedCurve::evaluate_speed_multiplier(double timeline_ratio) const noexcept {
    if (points_.empty()) {
        return 1.0;
    }

    const double ratio = std::clamp(timeline_ratio, 0.0, 1.0);

    if (ratio <= points_.front().timeline_ratio) {
        return points_.front().speed_multiplier;
    }

    if (ratio >= points_.back().timeline_ratio) {
        return points_.back().speed_multiplier;
    }

    auto it = std::upper_bound(points_.begin(), points_.end(), ratio,
        [](double r, const SpeedPoint& pt) { return r < pt.timeline_ratio; });

    if (it == points_.begin()) {
        return it->speed_multiplier;
    }

    const auto& left = *(it - 1);
    const auto& right = *it;

    const double span = right.timeline_ratio - left.timeline_ratio;
    if (span <= 1e-9) {
        return left.speed_multiplier;
    }

    const double t = (ratio - left.timeline_ratio) / span;
    return left.speed_multiplier + (right.speed_multiplier - left.speed_multiplier) * t;
}

int64_t SpeedCurve::map_timeline_to_source_offset(
    int64_t timeline_offset_ticks,
    int64_t timeline_duration_ticks,
    double constant_speed_fallback
) const noexcept {
    if (timeline_duration_ticks <= 0 || timeline_offset_ticks <= 0) {
        return 0;
    }

    if (points_.size() < 2) {
        return static_cast<int64_t>(std::round(static_cast<double>(timeline_offset_ticks) * constant_speed_fallback));
    }

    const double target_ratio = std::clamp(
        static_cast<double>(timeline_offset_ticks) / static_cast<double>(timeline_duration_ticks),
        0.0,
        1.0
    );

    double total_integral = 0.0;

    for (size_t i = 0; i + 1 < points_.size(); ++i) {
        const auto& p0 = points_[i];
        const auto& p1 = points_[i + 1];

        if (target_ratio <= p0.timeline_ratio) {
            break;
        }

        const double segment_end = std::min(target_ratio, p1.timeline_ratio);
        const double dt = segment_end - p0.timeline_ratio;
        if (dt <= 0.0) {
            continue;
        }

        const double span = p1.timeline_ratio - p0.timeline_ratio;
        if (span <= 1e-9) {
            continue;
        }

        const double m = (p1.speed_multiplier - p0.speed_multiplier) / span;
        // Integral of (p0.speed + m * t) dt = p0.speed * dt + 0.5 * m * dt^2
        const double segment_integral = (p0.speed_multiplier * dt) + (0.5 * m * dt * dt);
        total_integral += segment_integral;
    }

    const double mapped_ticks = total_integral * static_cast<double>(timeline_duration_ticks);
    return static_cast<int64_t>(std::round(mapped_ticks));
}

} // namespace opencut::speed
