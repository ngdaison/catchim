#include "opencut/speed.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut::speed;

static void test_constant_speed_fallback() {
    SpeedCurve curve;
    const int64_t dur = 120000;
    const int64_t off = 60000;

    // Normal speed 1.0x -> 60000
    assert(curve.map_timeline_to_source_offset(off, dur, 1.0) == 60000);
    // 2.0x speed -> 120000
    assert(curve.map_timeline_to_source_offset(off, dur, 2.0) == 120000);
    // 0.5x speed -> 30000
    assert(curve.map_timeline_to_source_offset(off, dur, 0.5) == 30000);
    std::cout << "[PASS] test_constant_speed_fallback\n";
}

static void test_speed_ramp() {
    SpeedCurve curve;
    // Ramps from 1.0x at start to 2.0x at end
    curve.add_point(0.0, 1.0);
    curve.add_point(1.0, 2.0);

    const int64_t dur = 120000;

    // At start: offset 0 -> 0
    assert(curve.map_timeline_to_source_offset(0, dur) == 0);

    // Speed at t=0.5 is 1.5x
    assert(std::abs(curve.evaluate_speed_multiplier(0.5) - 1.5) < 1e-6);

    // Average speed from 0 to 1 is 1.5. Total source duration = 120000 * 1.5 = 180000
    const int64_t end_offset = curve.map_timeline_to_source_offset(dur, dur);
    assert(std::abs(end_offset - 180000) <= 1);

    // Integral from 0 to 0.5: avg speed is 1.25. (120000 * 0.5) * 1.25 = 75000
    const int64_t mid_offset = curve.map_timeline_to_source_offset(60000, dur);
    assert(std::abs(mid_offset - 75000) <= 1);

    std::cout << "[PASS] test_speed_ramp\n";
}

int main() {
    test_constant_speed_fallback();
    test_speed_ramp();
    std::cout << "All speed tests passed successfully!\n";
    return 0;
}
