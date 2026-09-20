#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>

namespace catchim::render {

struct ColorWheelTones {
    double r{0.0};
    double g{0.0};
    double b{0.0};
    double master{0.0};
};

struct ColorWheelGrade {
    // Lift: offsets shadows [-1.0, 1.0], default 0.0
    ColorWheelTones lift{0.0, 0.0, 0.0, 0.0};
    // Gamma: power curve for midtones [0.1, 10.0], default 1.0
    ColorWheelTones gamma{1.0, 1.0, 1.0, 1.0};
    // Gain: multiplier for highlights [0.0, 4.0], default 1.0
    ColorWheelTones gain{1.0, 1.0, 1.0, 1.0};
    // Offset: linear offset [-1.0, 1.0], default 0.0
    ColorWheelTones offset{0.0, 0.0, 0.0, 0.0};

    bool isIdentity() const noexcept {
        return lift.r == 0.0 && lift.g == 0.0 && lift.b == 0.0 && lift.master == 0.0 &&
               gamma.r == 1.0 && gamma.g == 1.0 && gamma.b == 1.0 && gamma.master == 1.0 &&
               gain.r == 1.0 && gain.g == 1.0 && gain.b == 1.0 && gain.master == 1.0 &&
               offset.r == 0.0 && offset.g == 0.0 && offset.b == 0.0 && offset.master == 0.0;
    }
};

class ColorWheelEngine {
public:
    static void applyColorWheelGrading(
        uint8_t* rgba,
        size_t pixelCount,
        const ColorWheelGrade& grade
    ) noexcept;
};

} // namespace catchim::render
