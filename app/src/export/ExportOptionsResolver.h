#pragma once

#include "ExportSettings.h"
#include "editor/project/ProjectSettings.h"
#include <optional>
#include <string>
#include <vector>

namespace catchim::exporting {

class ExportOptionsResolver {
public:
    static int roundToEven(double value) noexcept;
    static int getResolutionHeight(const std::string& resolution) noexcept;

    static editor::CanvasSize resolveExportCanvasSize(
        editor::CanvasSize sourceSize,
        const std::string& resolution) noexcept;

    static std::string getExportMimeType(ExportFormat format);
    static std::string getExportMimeType(const std::string& format);

    static std::string getExportFileExtension(ExportFormat format);
    static std::string getExportFileExtension(const std::string& format);

    static const std::vector<std::string>& getSupportedResolutions();
    static const std::vector<std::string>& getSupportedFormats();
    static const std::vector<std::string>& getSupportedQualities();
};

} // namespace catchim::exporting
