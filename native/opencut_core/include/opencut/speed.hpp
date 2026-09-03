#pragma once

#include <cstdint>
#include <vector>

namespace opencut::speed {

struct SpeedPoint {
    double timeline_ratio = 0.0;     // Progress along clip in timeline [0.0, 1.0]
    double speed_multiplier = 1.0;   // Speed factor, e.g. 1.0 = normal, 0.5 = 2x slow-mo, 2.0 = 2x fast-forward
};

class SpeedCurve {
public:
    SpeedCurve() = default;
    explicit SpeedCurve(std::vector<SpeedPoint> points);

    void add_point(double timeline_ratio, double speed_multiplier);
    void clear();

    [[nodiscard]] size_t size() const noexcept { return points_.size(); }
    [[nodiscard]] bool empty() const noexcept { return points_.empty(); }
    [[nodiscard]] const std::vector<SpeedPoint>& points() const noexcept { return points_; }

    [[nodiscard]] double evaluate_speed_multiplier(double timeline_ratio) const noexcept;

    // Maps timeline elapsed offset to source media offset
    [[nodiscard]] int64_t map_timeline_to_source_offset(
        int64_t timeline_offset_ticks,
        int64_t timeline_duration_ticks,
        double constant_speed_fallback = 1.0
    ) const noexcept;

private:
    std::vector<SpeedPoint> points_;
    void sort_and_clean();
};

} // namespace opencut::speed
