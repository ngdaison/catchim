#include "opencut/compositor.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut;

void test_transforms() {
    QuadTransform t{
        .center_x = 100.0f,
        .center_y = 50.0f,
        .width = 200.0f,
        .height = 100.0f,
        .rotation_degrees = 0.0f,
        .flip_x = false,
        .flip_y = false,
    };

    Matrix3x3 mat = Matrix3x3::from_transform(t);
    Point2D center = mat.transform_point({0.0f, 0.0f});
    assert(std::abs(center.x - 100.0f) < 1e-4f);
    assert(std::abs(center.y - 50.0f) < 1e-4f);

    // Inverse test
    Matrix3x3 inv = mat.inverse();
    Point2D original = inv.transform_point(center);
    assert(std::abs(original.x) < 1e-4f);
    assert(std::abs(original.y) < 1e-4f);
}

void test_blending() {
    ColorRGBA base{0.5f, 0.5f, 0.5f, 1.0f};
    ColorRGBA layer{1.0f, 0.0f, 0.0f, 1.0f};

    // Normal blend with full opacity -> layer color
    ColorRGBA normal = blend_colors(base, layer, BlendMode::Normal, 1.0f);
    assert(std::abs(normal.r - 1.0f) < 1e-4f);
    assert(std::abs(normal.g - 0.0f) < 1e-4f);
    assert(std::abs(normal.b - 0.0f) < 1e-4f);

    // Multiply: 0.5 * 1.0 = 0.5 (r), 0.5 * 0.0 = 0.0 (g, b)
    ColorRGBA mult = blend_colors(base, layer, BlendMode::Multiply, 1.0f);
    assert(std::abs(mult.r - 0.5f) < 1e-4f);
    assert(std::abs(mult.g - 0.0f) < 1e-4f);
    assert(std::abs(mult.b - 0.0f) < 1e-4f);

    // Screen: 0.5 + 1.0 - 0.5 = 1.0 (r), 0.5 + 0.0 - 0.0 = 0.5 (g, b)
    ColorRGBA scr = blend_colors(base, layer, BlendMode::Screen, 1.0f);
    assert(std::abs(scr.r - 1.0f) < 1e-4f);
    assert(std::abs(scr.g - 0.5f) < 1e-4f);
    assert(std::abs(scr.b - 0.5f) < 1e-4f);
}

int main() {
    test_transforms();
    test_blending();
    std::cout << "All compositor tests passed successfully!\n";
    return 0;
}
