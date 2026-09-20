#include "ExportOptionsResolver.h"

#include <algorithm>
#include <cmath>

namespace catchim::exporting {

int ExportOptionsResolver::roundToEven(double value) noexcept {
    return std::max(2, static_cast<int>(std::round(value / 2.0) * 2.0));
}

int ExportOptionsResolver::getResolutionHeight(const std::string& resolution) noexcept {
    if (resolution == "480p") return 480;
    if (resolution == "720p") return 720;
    if (resolution == "1080p") return 1080;
    if (resolution == "1440p") return 1440;
    if (resolution == "2160p") return 2160;
    if (resolution == "4320p") return 4320;
    return 0; // "source" or unknown
}

editor::CanvasSize ExportOptionsResolver::resolveExportCanvasSize(
    editor::CanvasSize sourceSize,
    const std::string& resolution) noexcept {

    if (resolution.empty() || resolution == "source") {
        return sourceSize;
    }

    const int targetHeight = getResolutionHeight(resolution);
    if (targetHeight <= 0 || sourceSize.height <= 0) {
        return sourceSize;
    }

    const double aspectRatio = static_cast<double>(sourceSize.width) / static_cast<double>(sourceSize.height);
    const int resolvedWidth = roundToEven(static_cast<double>(targetHeight) * aspectRatio);

    return editor::CanvasSize{resolvedWidth, targetHeight};
}

std::string ExportOptionsResolver::getExportMimeType(ExportFormat format) {
    switch (format) {
    case ExportFormat::MP4:
        return "video/mp4";
    case ExportFormat::WebM:
        return "video/webm";
    case ExportFormat::FrameSequence:
        return "image/png";
    }
    return "video/mp4";
}

std::string ExportOptionsResolver::getExportMimeType(const std::string& format) {
    if (format == "webm") {
        return "video/webm";
    }
    return "video/mp4";
}

std::string ExportOptionsResolver::getExportFileExtension(ExportFormat format) {
    switch (format) {
    case ExportFormat::MP4:
        return ".mp4";
    case ExportFormat::WebM:
        return ".webm";
    case ExportFormat::FrameSequence:
        return ".png";
    }
    return ".mp4";
}

std::string ExportOptionsResolver::getExportFileExtension(const std::string& format) {
    if (format.empty()) return ".mp4";
    if (format[0] == '.') return format;
    return "." + format;
}

const std::vector<std::string>& ExportOptionsResolver::getSupportedResolutions() {
    static const std::vector<std::string> s_resolutions = {
        "source", "480p", "720p", "1080p", "1440p", "2160p", "4320p"
    };
    return s_resolutions;
}

const std::vector<std::string>& ExportOptionsResolver::getSupportedFormats() {
    static const std::vector<std::string> s_formats = {
        "mp4", "webm"
    };
    return s_formats;
}

const std::vector<std::string>& ExportOptionsResolver::getSupportedQualities() {
    static const std::vector<std::string> s_qualities = {
        "low", "medium", "high", "very_high"
    };
    return s_qualities;
}

} // namespace catchim::exporting
