#pragma once

#include "editor/timeline/Track.h"
#include <string>
#include <string_view>
#include <optional>

namespace catchim::editor {

struct TrackThemeInfo {
    std::string elementClassName;
    std::string hexColor;
    std::optional<std::string> waveformColor{std::nullopt};
};

class TimelineTheme {
public:
    static constexpr std::string_view TIMELINE_AUDIO_WAVEFORM_COLOR = "rgba(255, 255, 255, 0.7)";
    static constexpr std::string_view SELECTED_TRACK_ROW_CLASS = "bg-accent/50";
    static constexpr std::string_view DEFAULT_TIMELINE_BOOKMARK_COLOR = "#009dff";

    static TrackThemeInfo getTrackTheme(TrackType type) noexcept;
    static std::string_view getTimelineElementClassName(TrackType type) noexcept;
    static std::string_view getTrackHexColor(TrackType type) noexcept;
};

} // namespace catchim::editor
