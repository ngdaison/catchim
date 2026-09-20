#pragma once

#include <string>
#include <string_view>

namespace catchim::editor {

struct ExportMimeTypes {
    static constexpr std::string_view WEBM = "video/webm";
    static constexpr std::string_view MP4 = "video/mp4";

    static std::string_view getMimeTypeForExtension(std::string_view ext) noexcept;
    static std::string_view getExtensionForMimeType(std::string_view mime) noexcept;
};

struct TimelineLayers {
    static constexpr int TRACK_CONTENT = 10;
    static constexpr int DRAG_LINE = 20;
    static constexpr int PLAYHEAD = 30;
    static constexpr int SNAP_INDICATOR = 40;
};

struct PreviewPenCursor {
    static constexpr int HOTSPOT_X = 1;
    static constexpr int HOTSPOT_Y = 1;

    static const char* getSvgContent() noexcept;
};

} // namespace catchim::editor
