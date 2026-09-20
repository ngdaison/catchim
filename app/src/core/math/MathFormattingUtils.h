#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace catchim::core {

class MathFormattingUtils {
public:
    static double clamp(double value, double min, double max) noexcept;
    static int64_t clampRound(double value, double min, double max) noexcept;

    static int getFractionDigitsForStep(double step) noexcept;
    static double snapToStep(double value, double step) noexcept;

    static bool isNearlyEqual(double left, double right, double epsilon = 0.0001) noexcept;

    static std::string formatNumberForDisplay(
        double value,
        std::optional<int> fractionDigits = std::nullopt,
        int minFractionDigits = 0,
        int maxFractionDigits = 6
    );

    static std::string dimensionToAspectRatio(int width, int height);
};

} // namespace catchim::core
