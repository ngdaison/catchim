#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <cmath>

namespace catchim::core {

class MathExpressionEvaluator {
public:
    // Safely parses and evaluates basic arithmetic expressions (+, -, *, /, parentheses, decimals)
    // Returns std::nullopt on syntax error, division by zero, or invalid characters.
    static std::optional<double> evaluateMathExpression(std::string_view input);

    // Snaps a value to the nearest step increment (e.g. step=0.1 -> snaps to tenths)
    static double snapToStep(double value, double step) noexcept;

    // Formats a number for inspector/UI display without trailing redundant zeros
    static std::string formatNumberForDisplay(
        double value,
        int minFractionDigits = 0,
        int maxFractionDigits = 6
    );

    // Floating-point equality with epsilon tolerance
    static bool isNearlyEqual(double a, double b, double epsilon = 0.0001) noexcept;
};

} // namespace catchim::core
