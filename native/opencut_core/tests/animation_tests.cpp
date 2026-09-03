#include "opencut/animation.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut::animation;

static void test_cubic_bezier_linear() {
    // Linear curve (p0=0, p1=1/3, p2=2/3, p3=1)
    const double pt0 = CubicBezier::evaluate_point(0.0, 0.0, 1.0/3.0, 2.0/3.0, 1.0);
    assert(std::abs(pt0 - 0.0) < 1e-6);

    const double pt5 = CubicBezier::evaluate_point(0.5, 0.0, 1.0/3.0, 2.0/3.0, 1.0);
    assert(std::abs(pt5 - 0.5) < 1e-6);

    const double pt1 = CubicBezier::evaluate_point(1.0, 0.0, 1.0/3.0, 2.0/3.0, 1.0);
    assert(std::abs(pt1 - 1.0) < 1e-6);

    const double prog = CubicBezier::solve_progress_for_time(0.5, 0.0, 1.0/3.0, 2.0/3.0, 1.0);
    assert(std::abs(prog - 0.5) < 1e-3);
    std::cout << "[PASS] test_cubic_bezier_linear\n";
}

static void test_keyframe_channel_linear() {
    KeyframeChannel channel(0.0);
    channel.insert_or_update_keyframe(Keyframe{0, 100.0, InterpolationType::Linear});
    channel.insert_or_update_keyframe(Keyframe{120000, 200.0, InterpolationType::Linear});

    assert(std::abs(channel.evaluate(0) - 100.0) < 1e-6);
    assert(std::abs(channel.evaluate(60000) - 150.0) < 1e-6);
    assert(std::abs(channel.evaluate(120000) - 200.0) < 1e-6);
    assert(std::abs(channel.evaluate(-1000) - 100.0) < 1e-6);
    assert(std::abs(channel.evaluate(200000) - 200.0) < 1e-6);
    std::cout << "[PASS] test_keyframe_channel_linear\n";
}

static void test_keyframe_channel_hold() {
    KeyframeChannel channel(0.0);
    channel.insert_or_update_keyframe(Keyframe{0, 50.0, InterpolationType::Hold});
    channel.insert_or_update_keyframe(Keyframe{120000, 150.0, InterpolationType::Hold});

    assert(std::abs(channel.evaluate(0) - 50.0) < 1e-6);
    assert(std::abs(channel.evaluate(60000) - 50.0) < 1e-6);
    assert(std::abs(channel.evaluate(119999) - 50.0) < 1e-6);
    assert(std::abs(channel.evaluate(120000) - 150.0) < 1e-6);
    std::cout << "[PASS] test_keyframe_channel_hold\n";
}

static void test_keyframe_channel_bezier() {
    KeyframeChannel channel(0.0);
    Keyframe k1{0, 0.0, InterpolationType::Bezier};
    Keyframe k2{120000, 100.0, InterpolationType::Bezier};
    channel.insert_or_update_keyframe(k1);
    channel.insert_or_update_keyframe(k2);

    const double mid_val = channel.evaluate(60000);
    assert(mid_val > 0.0 && mid_val < 100.0);
    assert(std::abs(mid_val - 50.0) < 1.0);
    std::cout << "[PASS] test_keyframe_channel_bezier\n";
}

static void test_transform_group() {
    TransformChannelGroup group;
    group.position_x.insert_or_update_keyframe(Keyframe{0, 0.0, InterpolationType::Linear});
    group.position_x.insert_or_update_keyframe(Keyframe{120000, 400.0, InterpolationType::Linear});

    const auto t = group.evaluate(60000);
    assert(std::abs(t.position_x - 200.0) < 1e-6);
    assert(std::abs(t.scale_x - 1.0) < 1e-6);
    assert(std::abs(t.opacity - 1.0) < 1e-6);
    std::cout << "[PASS] test_transform_group\n";
}

int main() {
    test_cubic_bezier_linear();
    test_keyframe_channel_linear();
    test_keyframe_channel_hold();
    test_keyframe_channel_bezier();
    test_transform_group();
    std::cout << "All animation tests passed successfully!\n";
    return 0;
}
