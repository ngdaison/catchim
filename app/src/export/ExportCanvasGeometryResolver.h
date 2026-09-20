#pragma once

#include "export/ExportSettings.h"
#include "editor/project/ProjectSettings.h"
#include <string_view>
#include <string>

namespace catchim::exporting {

enum class ExportResolutionPreset {
    Source,
    P480,
    P720,
    P1080,
    P1440,
    P2160,
    P4320
};

class ExportCanvasGeometryResolver {
public:
    // Enforces even pixel dimensions (multiples of 2) required by video codecs
    static int roundToEven(double value) noexcept;

    // Resolves target export canvas dimensions for a given preset while preserving aspect ratio
    static editor::CanvasSize resolveExportCanvasSize(
        editor::CanvasSize sourceSize,
        ExportResolutionPreset preset
    ) noexcept;

    // Resolves export canvas dimensions for an arbitrary target height while preserving aspect ratio
    static editor::CanvasSize resolveExportCanvasSize(
        editor::CanvasSize sourceSize,
        int targetHeight
    ) noexcept;

    // Maps preset to standard height pixel value (or 0 for Source)
    static int getPresetTargetHeight(ExportResolutionPreset preset) noexcept;

    // MIME type associated with the export format ("video/mp4", "video/webm")
    static std::string_view getExportMimeType(ExportFormat format) noexcept;

    // File extension associated with the export format (".mp4", ".webm")
    static std::string_view getExportFileExtension(ExportFormat format) noexcept;

    // String conversion helpers
    static std::string_view resolutionPresetToString(ExportResolutionPreset preset) noexcept;
    static ExportResolutionPreset stringToResolutionPreset(std::string_view str) noexcept;
};

} // namespace catchim::exporting
