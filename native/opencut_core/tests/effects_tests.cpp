#include "opencut/effects.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut;

void test_color_adjustments() {
    ColorRGBA white{1.0f, 1.0f, 1.0f, 1.0f};

    // Invert
    ColorRGBA inv = invert_color(white);
    assert(std::abs(inv.r) < 1e-4f);
    assert(std::abs(inv.g) < 1e-4f);
    assert(std::abs(inv.b) < 1e-4f);
    assert(std::abs(inv.a - 1.0f) < 1e-4f);

    // Brightness increase
    ColorRGBA mid{0.5f, 0.5f, 0.5f, 1.0f};
    ColorAdjustments adj_b{.brightness = 0.2f};
    ColorRGBA brighter = apply_color_adjustments(mid, adj_b);
    assert(std::abs(brighter.r - 0.7f) < 1e-4f);

    // Vignette
    VignetteParams vparams{.amount = 1.0f, .softness = 0.5f, .roundness = 1.0f};
    float vig_center = calculate_vignette({0.5f, 0.5f}, vparams);
    assert(vig_center == 1.0f); // Center is unaffected

    // Gaussian kernel
    auto kernel = generate_gaussian_kernel_1d(2, 1.0f);
    assert(kernel.size() == 5);
    float sum = 0.0f;
    for (float w : kernel) sum += w;
    assert(std::abs(sum - 1.0f) < 1e-4f);
}

int main() {
    test_color_adjustments();
    std::cout << "All effects tests passed successfully!\n";
    return 0;
}
