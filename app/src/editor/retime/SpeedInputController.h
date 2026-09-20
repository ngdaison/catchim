#pragma once

#include "editor/retime/RetimeRateEngine.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <optional>

namespace catchim::editor {

class SpeedInputController {
public:
    static constexpr double SPEED_STEP = 0.01;
    static constexpr int SPEED_FRACTION_DIGITS = 2;

    static std::string rateToDisplay(double rate);

    static std::optional<double> parseSpeedInput(const std::string& input);

    static std::optional<RetimeConfig> buildRetime(double rate, bool maintainPitch);

    static bool canMaintainPitch(ClipType type) noexcept;
};

} // namespace catchim::editor
