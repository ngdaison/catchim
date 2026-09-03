#include "opencut/masks.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut;

void test_sdfs() {
    // Circle at origin with radius 10
    assert(sdf_circle({0.0f, 0.0f}, 10.0f) == -10.0f); // inside
    assert(sdf_circle({10.0f, 0.0f}, 10.0f) == 0.0f);   // on border
    assert(sdf_circle({15.0f, 0.0f}, 10.0f) == 5.0f);   // outside

    // Rectangle with half size 20, 10
    assert(sdf_rectangle({0.0f, 0.0f}, {20.0f, 10.0f}) == -10.0f); // inside
    assert(sdf_rectangle({20.0f, 0.0f}, {20.0f, 10.0f}) == 0.0f);
    assert(sdf_rectangle({25.0f, 0.0f}, {20.0f, 10.0f}) == 5.0f);
}

void test_mask_evaluation() {
    MaskDefinition rect_mask{
        .type = MaskShapeType::Rectangle,
        .center = {0.0f, 0.0f},
        .size = {40.0f, 20.0f},
        .rotation_degrees = 0.0f,
        .feather = 0.0f,
        .inverted = false,
    };

    assert(evaluate_mask_alpha({0.0f, 0.0f}, rect_mask) == 1.0f); // center is fully opaque
    assert(evaluate_mask_alpha({50.0f, 50.0f}, rect_mask) == 0.0f); // outside is fully transparent

    rect_mask.inverted = true;
    assert(evaluate_mask_alpha({0.0f, 0.0f}, rect_mask) == 0.0f);
    assert(evaluate_mask_alpha({50.0f, 50.0f}, rect_mask) == 1.0f);
}

int main() {
    test_sdfs();
    test_mask_evaluation();
    std::cout << "All mask tests passed successfully!\n";
    return 0;
}
