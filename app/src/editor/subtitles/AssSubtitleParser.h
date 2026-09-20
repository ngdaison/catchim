#pragma once

#include "core/time/TimelineTime.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>

namespace catchim::editor {

enum class SubtitleTextAlign {
    Left,
    Center,
    Right
};

enum class SubtitleVerticalAlign {
    Top,
    Middle,
    Bottom
};

struct SubtitlePlacement {
    SubtitleVerticalAlign verticalAlign{SubtitleVerticalAlign::Bottom};
    std::optional<double> marginLeftRatio;
    std::optional<double> marginRightRatio;
    std::optional<double> marginVerticalRatio;
};

struct SubtitleStyle {
    std::string name{"Default"};
    std::string fontFamily{"Arial"};
    double fontSize{20.0};
    double fontSizeRatioOfPlayHeight{20.0 / 288.0};
    std::string color{"#ffffff"};
    double opacity{1.0};
    SubtitleTextAlign textAlign{SubtitleTextAlign::Center};
    SubtitlePlacement placement;
    bool bold{false};
    bool italic{false};
    bool underline{false};
    double outlineWidth{0.0};
    std::string outlineColor{"#000000"};
    double shadowWidth{0.0};
};

struct AssSubtitleCue {
    std::string text;
    core::TimelineTime startTime{0};
    core::TimelineTime duration{0};
    SubtitleStyle style;

    [[nodiscard]] core::TimelineTime endTime() const noexcept {
        return startTime + duration;
    }
};

struct ParseAssResult {
    std::vector<AssSubtitleCue> cues;
    int32_t skippedCueCount{0};
    int32_t strippedInlineTagCueCount{0};
    int32_t ignoredEffectCount{0};
    int32_t missingStyleCueCount{0};
    std::vector<std::string> warnings;
};

class AssSubtitleParser {
public:
    static constexpr double DEFAULT_PLAY_RES_X = 384.0;
    static constexpr double DEFAULT_PLAY_RES_Y = 288.0;

    static ParseAssResult parse(const std::string& input);

    // Helper utilities
    static double parseTimestamp(const std::string& input);
    static std::string formatTimestamp(double seconds);
    static std::pair<std::string, double> parseColor(const std::string& assColor);
    static std::string stripAssText(const std::string& input, bool& outHadInlineTags);
};

} // namespace catchim::editor
