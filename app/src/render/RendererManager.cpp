#include "RendererManager.h"
#include "core/time/Timecode.h"
#include <sstream>
#include <algorithm>

namespace catchim::render {

void RendererManager::setDegraded(bool degraded) noexcept {
    if (isDegraded_ != degraded) {
        isDegraded_ = degraded;
        notify();
    }
}

SnapshotResult RendererManager::createSnapshot(
    core::TimelineTime renderTime,
    editor::CanvasSize canvasSize,
    const std::string& projectName
) {
    if (canvasSize.width <= 0 || canvasSize.height <= 0) {
        return SnapshotResult{
            .success = false,
            .filename = "",
            .error = "Invalid canvas dimensions",
            .width = 0,
            .height = 0
        };
    }

    std::string safeName = projectName.empty() ? "snapshot" : projectName;
    for (char& c : safeName) {
        if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' ||
            c == '\\' || c == '|' || c == '?' || c == '*') {
            c = '-';
        }
    }

    std::string tcStr = core::Timecode::format(renderTime);
    for (char& c : tcStr) {
        if (c == ':') c = '-';
    }

    std::string filename = safeName + "-" + tcStr + ".png";
    return SnapshotResult{
        .success = true,
        .filename = filename,
        .error = "",
        .width = canvasSize.width,
        .height = canvasSize.height
    };
}

exporting::ExportResult RendererManager::exportProject(
    const exporting::ExportSettings& options,
    core::TimelineTime duration,
    ProgressCallback onProgress,
    CancelCheckCallback onCancel
) {
    if (duration.ticks() <= 0) {
        return exporting::ExportResult{
            .success = false,
            .error = "Project is empty"
        };
    }

    if (onCancel && onCancel()) {
        return exporting::ExportResult{
            .success = false,
            .error = "Export cancelled by user",
            .isCancelled = true
        };
    }

    if (onProgress) {
        onProgress(0.05);
    }

    // Export progress simulation / steps
    const int totalSteps = 10;
    for (int step = 1; step <= totalSteps; ++step) {
        if (onCancel && onCancel()) {
            return exporting::ExportResult{
                .success = false,
                .error = "Export cancelled by user",
                .isCancelled = true
            };
        }
        if (onProgress) {
            double progress = 0.05 + 0.95 * (static_cast<double>(step) / totalSteps);
            onProgress(progress);
        }
    }

    exporting::ExportResult result;
    result.success = true;
    result.outputPath = options.outputPath;
    result.totalFrames = static_cast<int64_t>(duration.toSeconds() * options.fps.numerator / options.fps.denominator);
    result.durationSeconds = duration.toSeconds();
    return result;
}

void RendererManager::subscribe(RendererChangeListener listener) {
    listeners_.push_back(std::move(listener));
}

void RendererManager::notify() {
    for (const auto& l : listeners_) {
        if (l) l();
    }
}

} // namespace catchim::render
