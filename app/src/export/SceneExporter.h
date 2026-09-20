#pragma once

#include "ExportSettings.h"
#include "editor/timeline/Timeline.h"
#include "render/RenderEngine.h"
#include "audio/AudioMixer.h"
#include <functional>
#include <atomic>
#include <string>

namespace catchim::exporting {

using ProgressCallback = std::function<void(double progress)>;
using CompleteCallback = std::function<void(bool success, const std::string& message)>;

class SceneExporter {
public:
    SceneExporter(
        const editor::Timeline& timeline,
        ExportSettings settings
    );

    ~SceneExporter();

    void setProgressCallback(ProgressCallback cb) { m_progressCallback = std::move(cb); }
    void setCompleteCallback(CompleteCallback cb) { m_completeCallback = std::move(cb); }

    bool runExport();
    void cancel();

    bool isCancelled() const noexcept { return m_cancelled.load(); }
    int64_t totalFrames() const noexcept { return m_totalFrames; }
    int64_t renderedFrames() const noexcept { return m_renderedFrames; }

private:
    const editor::Timeline& m_timeline;
    ExportSettings m_settings;
    std::atomic<bool> m_cancelled{false};
    std::atomic<bool> m_running{false};

    int64_t m_totalFrames{0};
    int64_t m_renderedFrames{0};

    ProgressCallback m_progressCallback;
    CompleteCallback m_completeCallback;
};

} // namespace catchim::exporting
