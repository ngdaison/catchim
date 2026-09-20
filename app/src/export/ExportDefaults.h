#pragma once

#include <string>
#include <vector>

namespace catchim::exporting {

struct ExportPresetOptions {
    std::string format{"mp4"};
    std::string quality{"high"};
    std::string resolution{"source"};
    bool includeAudio{true};

    bool operator==(const ExportPresetOptions& other) const noexcept {
        return format == other.format &&
               quality == other.quality &&
               resolution == other.resolution &&
               includeAudio == other.includeAudio;
    }
    bool operator!=(const ExportPresetOptions& other) const noexcept {
        return !(*this == other);
    }
};

class ExportDefaults {
public:
    static const ExportPresetOptions& defaultExportOptions() noexcept;

    static const std::string& mimeTypeMp4() noexcept;
    static const std::string& mimeTypeWebm() noexcept;

    static std::string getMimeTypeForFormat(const std::string& format);

    static const std::vector<std::string>& supportedFormats() noexcept;
    static const std::vector<std::string>& qualityPresets() noexcept;
    static const std::vector<std::string>& resolutionPresets() noexcept;
};

} // namespace catchim::exporting
