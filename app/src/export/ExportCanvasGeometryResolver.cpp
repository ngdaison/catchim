#include "export/ExportCanvasGeometryResolver.h"
#include <cmath>
#include <algorithm>

namespace catchim::exporting {

int ExportCanvasGeometryResolver::roundToEven(double value) noexcept {
    return std::max(2, static_cast<int>(std::round(value / 2.0)) * 2);
}

int ExportCanvasGeometryResolver::getPresetTargetHeight(ExportResolutionPreset preset) noexcept {
    switch (preset) {
        case ExportResolutionPreset::Source: return 0;
        case ExportResolutionPreset::P480:   return 480;
        case ExportResolutionPreset::P720:   return 720;
        case ExportResolutionPreset::P1080:  return 1080;
        case ExportResolutionPreset::P1440:  return 1440;
        case ExportResolutionPreset::P2160:  return 2160;
        case ExportResolutionPreset::P4320:  return 4320;
    }
    return 0;
}

editor::CanvasSize ExportCanvasGeometryResolver::resolveExportCanvasSize(
    editor::CanvasSize sourceSize,
    ExportResolutionPreset preset
) noexcept {
    if (preset == ExportResolutionPreset::Source) {
        return sourceSize;
    }
    int targetHeight = getPresetTargetHeight(preset);
    return resolveExportCanvasSize(sourceSize, targetHeight);
}

editor::CanvasSize ExportCanvasGeometryResolver::resolveExportCanvasSize(
    editor::CanvasSize sourceSize,
    int targetHeight
) noexcept {
    if (targetHeight <= 0 || sourceSize.height <= 0) {
        return sourceSize;
    }

    double aspectRatio = static_cast<double>(sourceSize.width) / static_cast<double>(sourceSize.height);
    int targetWidth = roundToEven(static_cast<double>(targetHeight) * aspectRatio);

    return editor::CanvasSize{targetWidth, targetHeight};
}

std::string_view ExportCanvasGeometryResolver::getExportMimeType(ExportFormat format) noexcept {
    switch (format) {
        case ExportFormat::MP4:           return "video/mp4";
        case ExportFormat::WebM:          return "video/webm";
        case ExportFormat::FrameSequence: return "image/png";
    }
    return "video/mp4";
}

std::string_view ExportCanvasGeometryResolver::getExportFileExtension(ExportFormat format) noexcept {
    switch (format) {
        case ExportFormat::MP4:           return ".mp4";
        case ExportFormat::WebM:          return ".webm";
        case ExportFormat::FrameSequence: return ".png";
    }
    return ".mp4";
}

std::string_view ExportCanvasGeometryResolver::resolutionPresetToString(ExportResolutionPreset preset) noexcept {
    switch (preset) {
        case ExportResolutionPreset::Source: return "source";
        case ExportResolutionPreset::P480:   return "480p";
        case ExportResolutionPreset::P720:   return "720p";
        case ExportResolutionPreset::P1080:  return "1080p";
        case ExportResolutionPreset::P1440:  return "1440p";
        case ExportResolutionPreset::P2160:  return "2160p";
        case ExportResolutionPreset::P4320:  return "4320p";
    }
    return "source";
}

ExportResolutionPreset ExportCanvasGeometryResolver::stringToResolutionPreset(std::string_view str) noexcept {
    if (str == "480p") return ExportResolutionPreset::P480;
    if (str == "720p") return ExportResolutionPreset::P720;
    if (str == "1080p") return ExportResolutionPreset::P1080;
    if (str == "1440p") return ExportResolutionPreset::P1440;
    if (str == "2160p") return ExportResolutionPreset::P2160;
    if (str == "4320p") return ExportResolutionPreset::P4320;
    return ExportResolutionPreset::Source;
}

} // namespace catchim::exporting
