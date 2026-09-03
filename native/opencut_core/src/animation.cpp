#include "opencut/animation.hpp"
#include <algorithm>
#include <cmath>

namespace opencut::animation {

double CubicBezier::evaluate_point(double progress, double p0, double p1, double p2, double p3) noexcept {
    const double mt = 1.0 - progress;
    const double mt2 = mt * mt;
    const double p2_term = progress * progress;
    return (mt2 * mt * p0) +
           (3.0 * mt2 * progress * p1) +
           (3.0 * mt * p2_term * p2) +
           (p2_term * progress * p3);
}

double CubicBezier::solve_progress_for_time(double time, double t0, double t1, double t2, double t3, int max_iterations) noexcept {
    double lower = 0.0;
    double upper = 1.0;

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        const double mid = (lower + upper) * 0.5;
        const double estimate = evaluate_point(mid, t0, t1, t2, t3);
        if (estimate < time) {
            lower = mid;
        } else {
            upper = mid;
        }
    }

    return (lower + upper) * 0.5;
}

void KeyframeChannel::insert_or_update_keyframe(const Keyframe& keyframe) {
    auto it = std::lower_bound(keyframes_.begin(), keyframes_.end(), keyframe.time,
        [](const Keyframe& k, int64_t t) { return k.time < t; });

    if (it != keyframes_.end() && it->time == keyframe.time) {
        *it = keyframe;
    } else {
        keyframes_.insert(it, keyframe);
    }
}

bool KeyframeChannel::remove_keyframe(int64_t time) {
    auto it = std::lower_bound(keyframes_.begin(), keyframes_.end(), time,
        [](const Keyframe& k, int64_t t) { return k.time < t; });

    if (it != keyframes_.end() && it->time == time) {
        keyframes_.erase(it);
        return true;
    }
    return false;
}

void KeyframeChannel::clear() {
    keyframes_.clear();
}

double KeyframeChannel::evaluate(int64_t time) const noexcept {
    if (keyframes_.empty()) {
        return default_value_;
    }

    if (time <= keyframes_.front().time) {
        return keyframes_.front().value;
    }

    if (time >= keyframes_.back().time) {
        return keyframes_.back().value;
    }

    // Binary search for the first keyframe with time > target
    auto right_it = std::upper_bound(keyframes_.begin(), keyframes_.end(), time,
        [](int64_t t, const Keyframe& k) { return t < k.time; });

    if (right_it == keyframes_.begin()) {
        return right_it->value;
    }

    auto left_it = right_it - 1;
    const auto& left = *left_it;
    const auto& right = *right_it;

    if (left.time == right.time) {
        return left.value;
    }

    switch (left.interpolation) {
        case InterpolationType::Hold:
            return left.value;

        case InterpolationType::Linear: {
            const double progress = static_cast<double>(time - left.time) / static_cast<double>(right.time - left.time);
            return left.value + (right.value - left.value) * progress;
        }

        case InterpolationType::Bezier: {
            const double span = static_cast<double>(right.time - left.time);
            const double value_delta = right.value - left.value;

            Handle2D right_handle = left.has_right_handle ? left.right_handle : Handle2D{span / 3.0, value_delta / 3.0};
            Handle2D left_handle = right.has_left_handle ? right.left_handle : Handle2D{-span / 3.0, -value_delta / 3.0};

            const double t0 = static_cast<double>(left.time);
            const double t1 = t0 + right_handle.dt;
            const double t2 = static_cast<double>(right.time) + left_handle.dt;
            const double t3 = static_cast<double>(right.time);

            const double progress = CubicBezier::solve_progress_for_time(static_cast<double>(time), t0, t1, t2, t3);

            const double v0 = left.value;
            const double v1 = v0 + right_handle.dv;
            const double v2 = right.value + left_handle.dv;
            const double v3 = right.value;

            return CubicBezier::evaluate_point(progress, v0, v1, v2, v3);
        }
    }

    return left.value;
}

TransformChannelGroup::TransformChannelGroup()
    : position_x(0.0),
      position_y(0.0),
      scale_x(1.0),
      scale_y(1.0),
      rotation_degrees(0.0),
      opacity(1.0),
      anchor_x(0.0),
      anchor_y(0.0) {}

Transform2DValues TransformChannelGroup::evaluate(int64_t time) const noexcept {
    return Transform2DValues{
        position_x.evaluate(time),
        position_y.evaluate(time),
        scale_x.evaluate(time),
        scale_y.evaluate(time),
        rotation_degrees.evaluate(time),
        opacity.evaluate(time),
        anchor_x.evaluate(time),
        anchor_y.evaluate(time)
    };
}

} // namespace opencut::animation
