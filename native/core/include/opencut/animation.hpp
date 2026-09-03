#pragma once

#include <cstdint>
#include <vector>
#include <optional>
#include "opencut/compositor.hpp"

namespace opencut::animation {

enum class InterpolationType {
    Hold = 0,
    Linear = 1,
    Bezier = 2
};

struct Handle2D {
    double dt = 0.0;
    double dv = 0.0;
};

struct Keyframe {
    int64_t time = 0; // MediaTime ticks
    double value = 0.0;
    InterpolationType interpolation = InterpolationType::Linear;
    Handle2D left_handle{};
    Handle2D right_handle{};
    bool has_left_handle = false;
    bool has_right_handle = false;
};

class CubicBezier {
public:
    static double evaluate_point(double progress, double p0, double p1, double p2, double p3) noexcept;
    static double solve_progress_for_time(double time, double t0, double t1, double t2, double t3, int max_iterations = 20) noexcept;
};

class KeyframeChannel {
public:
    KeyframeChannel() = default;
    explicit KeyframeChannel(double default_value) : default_value_(default_value) {}

    void insert_or_update_keyframe(const Keyframe& keyframe);
    bool remove_keyframe(int64_t time);
    void clear();

    [[nodiscard]] size_t size() const noexcept { return keyframes_.size(); }
    [[nodiscard]] bool empty() const noexcept { return keyframes_.empty(); }
    [[nodiscard]] const std::vector<Keyframe>& keyframes() const noexcept { return keyframes_; }

    [[nodiscard]] double evaluate(int64_t time) const noexcept;

private:
    double default_value_ = 0.0;
    std::vector<Keyframe> keyframes_;
};

struct Transform2DValues {
    double position_x = 0.0;
    double position_y = 0.0;
    double scale_x = 1.0;
    double scale_y = 1.0;
    double rotation_degrees = 0.0;
    double opacity = 1.0;
    double anchor_x = 0.0;
    double anchor_y = 0.0;
};

class TransformChannelGroup {
public:
    TransformChannelGroup();

    KeyframeChannel position_x;
    KeyframeChannel position_y;
    KeyframeChannel scale_x;
    KeyframeChannel scale_y;
    KeyframeChannel rotation_degrees;
    KeyframeChannel opacity;
    KeyframeChannel anchor_x;
    KeyframeChannel anchor_y;

    [[nodiscard]] Transform2DValues evaluate(int64_t time) const noexcept;
};

} // namespace opencut::animation
