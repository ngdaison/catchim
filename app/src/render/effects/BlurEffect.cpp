#include "BlurEffect.h"
#include <cmath>
#include <algorithm>
#include <numbers>

namespace catchim::render {

float BlurEffect::intensityToSigma(float intensity, float resolution, float reference) {
    if (reference <= 0.0f) return 0.0f;
    return (intensity / INTENSITY_TO_SIGMA_DIVISOR) * (resolution / reference);
}

std::vector<EffectPass> BlurEffect::buildGaussianBlurPasses(float sigmaX, float sigmaY) {
    float maxSigma = std::max(sigmaX, sigmaY);
    if (maxSigma < 0.001f) {
        return {};
    }

    int iterations = std::min(
        MAX_ITERATIONS,
        std::max(
            1,
            static_cast<int>(std::ceil(
                (maxSigma * maxSigma) / (MAX_EFFECTIVE_SIGMA * MAX_EFFECTIVE_SIGMA)
            ))
        )
    );

    float perPassSigmaX = sigmaX / std::sqrt(static_cast<float>(iterations));
    float perPassSigmaY = sigmaY / std::sqrt(static_cast<float>(iterations));
    float stepX = std::max(1.0f, perPassSigmaX / MAX_SINGLE_PASS_SIGMA);
    float stepY = std::max(1.0f, perPassSigmaY / MAX_SINGLE_PASS_SIGMA);

    std::vector<EffectPass> passes;
    passes.reserve(static_cast<size_t>(iterations) * 2);

    for (int i = 0; i < iterations; ++i) {
        // Horizontal pass
        EffectPass passH;
        passH.shader = GAUSSIAN_BLUR_SHADER;
        passH.sigma = perPassSigmaX;
        passH.step = stepX;
        passH.direction = {1.0f, 0.0f};
        passes.push_back(std::move(passH));

        // Vertical pass
        EffectPass passV;
        passV.shader = GAUSSIAN_BLUR_SHADER;
        passV.sigma = perPassSigmaY;
        passV.step = stepY;
        passV.direction = {0.0f, 1.0f};
        passes.push_back(std::move(passV));
    }

    return passes;
}

std::vector<float> BlurEffect::compute1DGaussianKernel(float sigma, int radius) {
    if (radius < 0) return {};
    if (radius == 0) return {1.0f};
    if (sigma <= 0.0001f) {
        std::vector<float> delta(static_cast<size_t>(2 * radius + 1), 0.0f);
        delta[static_cast<size_t>(radius)] = 1.0f;
        return delta;
    }

    int size = 2 * radius + 1;
    std::vector<float> kernel(static_cast<size_t>(size));
    float twoSigmaSq = 2.0f * sigma * sigma;
    float sum = 0.0f;

    for (int i = -radius; i <= radius; ++i) {
        float x = static_cast<float>(i);
        float weight = std::exp(-(x * x) / twoSigmaSq);
        kernel[static_cast<size_t>(i + radius)] = weight;
        sum += weight;
    }

    // Normalize sum to 1.0
    if (sum > 0.0f) {
        for (auto& w : kernel) {
            w /= sum;
        }
    }

    return kernel;
}

} // namespace catchim::render
