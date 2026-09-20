#include "export/ExportDefaults.h"

namespace catchim::exporting {

static const ExportPresetOptions s_defaultOptions{
    "mp4",
    "high",
    "source",
    true
};

static const std::string s_mimeMp4 = "video/mp4";
static const std::string s_mimeWebm = "video/webm";

static const std::vector<std::string> s_formats = { "mp4", "webm" };
static const std::vector<std::string> s_qualities = { "draft", "standard", "high", "lossless" };
static const std::vector<std::string> s_resolutions = { "source", "4k", "1080p", "720p", "480p" };

const ExportPresetOptions& ExportDefaults::defaultExportOptions() noexcept {
    return s_defaultOptions;
}

const std::string& ExportDefaults::mimeTypeMp4() noexcept {
    return s_mimeMp4;
}

const std::string& ExportDefaults::mimeTypeWebm() noexcept {
    return s_mimeWebm;
}

std::string ExportDefaults::getMimeTypeForFormat(const std::string& format) {
    if (format == "webm") return s_mimeWebm;
    return s_mimeMp4;
}

const std::vector<std::string>& ExportDefaults::supportedFormats() noexcept {
    return s_formats;
}

const std::vector<std::string>& ExportDefaults::qualityPresets() noexcept {
    return s_qualities;
}

const std::vector<std::string>& ExportDefaults::resolutionPresets() noexcept {
    return s_resolutions;
}

} // namespace catchim::exporting
