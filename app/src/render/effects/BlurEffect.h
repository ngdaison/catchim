#pragma once

#include <string>
#include <vector>
#include <utility>

namespace catchim::render {

struct EffectPass {
    std::string shader;
    float sigma{0.0f};
    float step{1.0f};
    std::pair<float, float> direction{1.0f, 0.0f};
};

/**
 * @brief Multi-pass Gaussian blur pipeline and discrete kernel calculator.
 * Corresponds to web/src/effects/definitions/blur.ts.
 */
class BlurEffect {
public:
    static constexpr const char* GAUSSIAN_BLUR_SHADER = "gaussian-blur";
    static constexpr float INTENSITY_TO_SIGMA_DIVISOR = 5.0f;
    static constexpr float MAX_SINGLE_PASS_SIGMA = 10.0f;
    static constexpr float MAX_STEP = 4.0f;
    static constexpr float MAX_EFFECTIVE_SIGMA = MAX_SINGLE_PASS_SIGMA * MAX_STEP; // 40.0f
    static constexpr int MAX_ITERATIONS = 8;

    /**
     * @brief Converts UI blur intensity [0..100] to shader sigma relative to resolution.
     */
    static float intensityToSigma(float intensity, float resolution, float reference = 1920.0f);

    /**
     * @brief Builds separable horizontal and vertical shader passes for Gaussian blur.
     */
    static std::vector<EffectPass> buildGaussianBlurPasses(float sigmaX, float sigmaY);

    /**
     * @brief Computes normalized 1D discrete Gaussian filter weights of size (2 * radius + 1).
     */
    static std::vector<float> compute1DGaussianKernel(float sigma, int radius);
};

} // namespace catchim::render
