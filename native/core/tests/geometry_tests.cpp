#include "opencut/geometry.hpp"
#include <cassert>
#include <iostream>

using namespace opencut::geometry;

static void test_point_in_rotated_rect() {
    // 100x50 rect centered at (200, 200), rotated 45 degrees
    // Center point (200, 200) must be inside
    assert(GeometryEngine::point_in_rotated_rect(200, 200, 200, 200, 100, 50, 45));

    // Point far away must be outside
    assert(!GeometryEngine::point_in_rotated_rect(0, 0, 200, 200, 100, 50, 45));

    // When rotation is 0, (240, 200) is inside (half-width is 50)
    assert(GeometryEngine::point_in_rotated_rect(240, 200, 200, 200, 100, 50, 0));
    // (260, 200) is outside
    assert(!GeometryEngine::point_in_rotated_rect(260, 200, 200, 200, 100, 50, 0));

    std::cout << "[PASS] test_point_in_rotated_rect\n";
}

static void test_snap_axis() {
    double snapped = 0.0;
    double delta = 0.0;

    // Source 102, target 100, threshold 5 -> snaps to 100 with delta -2
    bool snapped_res = GeometryEngine::test_snap_axis(102, 100, 5, &snapped, &delta);
    assert(snapped_res);
    assert(snapped == 100.0);
    assert(delta == -2.0);

    // Source 110, target 100, threshold 5 -> does not snap
    assert(!GeometryEngine::test_snap_axis(110, 100, 5, &snapped, &delta));

    std::cout << "[PASS] test_snap_axis\n";
}

int main() {
    test_point_in_rotated_rect();
    test_snap_axis();
    std::cout << "All geometry tests passed successfully!\n";
    return 0;
}
