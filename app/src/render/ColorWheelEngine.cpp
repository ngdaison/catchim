#include "render/ColorWheelEngine.h"
#include <cmath>
#include <array>

namespace catchim::render {

void ColorWheelEngine::applyColorWheelGrading(
    uint8_t* rgba,
    size_t pixelCount,
    const ColorWheelGrade& grade
) noexcept {
    if (rgba == nullptr || pixelCount == 0 || grade.isIdentity()) {
        return;
    }

    auto buildLut = [](double lift, double gamma, double gain, double offset) {
        std::array<uint8_t, 256> lut;
        gamma = std::max(0.01, gamma);
        gain = std::max(0.0, gain);

        for (int i = 0; i < 256; ++i) {
            double x = static_cast<double>(i) / 255.0;
            // 1. Lift
            double x1 = x * (1.0 - lift) + lift;
            // 2. Gain
            double x2 = x1 * gain;
            // 3. Gamma
            double x3 = (x2 > 0.0) ? std::pow(x2, 1.0 / gamma) : 0.0;
            // 4. Offset
            double x4 = x3 + offset;

            lut[i] = static_cast<uint8_t>(std::clamp(std::round(x4 * 255.0), 0.0, 255.0));
        }
        return lut;
    };

    auto lutR = buildLut(
        grade.lift.r + grade.lift.master,
        grade.gamma.r * grade.gamma.master,
        grade.gain.r * grade.gain.master,
        grade.offset.r + grade.offset.master
    );

    auto lutG = buildLut(
        grade.lift.g + grade.lift.master,
        grade.gamma.g * grade.gamma.master,
        grade.gain.g * grade.gain.master,
        grade.offset.g + grade.offset.master
    );

    auto lutB = buildLut(
        grade.lift.b + grade.lift.master,
        grade.gamma.b * grade.gamma.master,
        grade.gain.b * grade.gain.master,
        grade.offset.b + grade.offset.master
    );

    for (size_t i = 0; i < pixelCount; ++i) {
        size_t idx = i * 4;
        rgba[idx + 0] = lutR[rgba[idx + 0]];
        rgba[idx + 1] = lutG[rgba[idx + 1]];
        rgba[idx + 2] = lutB[rgba[idx + 2]];
        // Alpha unchanged
    }
}

} // namespace catchim::render
