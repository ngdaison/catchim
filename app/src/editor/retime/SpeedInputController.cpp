#include "SpeedInputController.h"
#include <iomanip>
#include <sstream>
#include <cmath>

namespace catchim::editor {

std::string SpeedInputController::rateToDisplay(double rate) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(SPEED_FRACTION_DIGITS) << rate;
    return ss.str();
}

std::optional<double> SpeedInputController::parseSpeedInput(const std::string& input) {
    if (input.empty()) return std::nullopt;

    try {
        size_t idx = 0;
        double parsed = std::stod(input, &idx);
        if (std::isnan(parsed) || std::isinf(parsed)) {
            return std::nullopt;
        }
        double snapped = std::round(parsed / SPEED_STEP) * SPEED_STEP;
        return RetimeRateEngine::clampRetimeRate(snapped);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<RetimeConfig> SpeedInputController::buildRetime(double rate, bool maintainPitch) {
    if (std::abs(rate - RetimeRateEngine::DEFAULT_RETIME_RATE) < 0.0001 && !maintainPitch) {
        return std::nullopt;
    }
    return RetimeRateEngine::buildConstantRetime(rate, maintainPitch);
}

bool SpeedInputController::canMaintainPitch(ClipType type) noexcept {
    return type == ClipType::Audio || type == ClipType::Video;
}

} // namespace catchim::editor
