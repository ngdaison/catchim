#pragma once

#include "editor/project/ProjectSettings.h"
#include "core/time/TimelineTime.h"
#include "export/ExportSettings.h"
#include <vector>
#include <string>
#include <functional>
#include <optional>

namespace catchim::render {

struct SnapshotResult {
    bool success{false};
    std::string filename;
    std::string error;
    int width{0};
    int height{0};
};

class RendererManager {
public:
    using RendererChangeListener = std::function<void()>;
    using ProgressCallback = std::function<void(double)>;
    using CancelCheckCallback = std::function<bool()>;

    RendererManager() = default;

    bool isDegraded() const noexcept { return isDegraded_; }
    void setDegraded(bool degraded) noexcept;

    SnapshotResult createSnapshot(
        core::TimelineTime renderTime,
        editor::CanvasSize canvasSize,
        const std::string& projectName = "snapshot"
    );

    exporting::ExportResult exportProject(
        const exporting::ExportSettings& options,
        core::TimelineTime duration,
        ProgressCallback onProgress = nullptr,
        CancelCheckCallback onCancel = nullptr
    );

    void subscribe(RendererChangeListener listener);

private:
    void notify();

    bool isDegraded_{false};
    std::vector<RendererChangeListener> listeners_;
};

} // namespace catchim::render
