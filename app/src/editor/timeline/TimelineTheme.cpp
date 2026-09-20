#include "TimelineTheme.h"

namespace catchim::editor {

TrackThemeInfo TimelineTheme::getTrackTheme(TrackType type) noexcept {
    switch (type) {
        case TrackType::Video:
            return TrackThemeInfo{
                .elementClassName = "transparent",
                .hexColor = "#00000000",
                .waveformColor = std::nullopt
            };
        case TrackType::Text:
            return TrackThemeInfo{
                .elementClassName = "bg-[#5DBAA0]",
                .hexColor = "#5DBAA0",
                .waveformColor = std::nullopt
            };
        case TrackType::Audio:
            return TrackThemeInfo{
                .elementClassName = "bg-[#8F5DBA]",
                .hexColor = "#8F5DBA",
                .waveformColor = std::string(TIMELINE_AUDIO_WAVEFORM_COLOR)
            };
        case TrackType::Graphic:
            return TrackThemeInfo{
                .elementClassName = "bg-[#BA5D7A]",
                .hexColor = "#BA5D7A",
                .waveformColor = std::nullopt
            };
        case TrackType::Effect:
            return TrackThemeInfo{
                .elementClassName = "bg-[#5d93ba]",
                .hexColor = "#5d93ba",
                .waveformColor = std::nullopt
            };
    }
    return TrackThemeInfo{
        .elementClassName = "transparent",
        .hexColor = "#00000000",
        .waveformColor = std::nullopt
    };
}

std::string_view TimelineTheme::getTimelineElementClassName(TrackType type) noexcept {
    switch (type) {
        case TrackType::Video: return "transparent";
        case TrackType::Text: return "bg-[#5DBAA0]";
        case TrackType::Audio: return "bg-[#8F5DBA]";
        case TrackType::Graphic: return "bg-[#BA5D7A]";
        case TrackType::Effect: return "bg-[#5d93ba]";
    }
    return "transparent";
}

std::string_view TimelineTheme::getTrackHexColor(TrackType type) noexcept {
    switch (type) {
        case TrackType::Video: return "#00000000";
        case TrackType::Text: return "#5DBAA0";
        case TrackType::Audio: return "#8F5DBA";
        case TrackType::Graphic: return "#BA5D7A";
        case TrackType::Effect: return "#5d93ba";
    }
    return "#00000000";
}

} // namespace catchim::editor
