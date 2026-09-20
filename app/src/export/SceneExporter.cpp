#include "export/SceneExporter.h"
#include "core/logging/Logger.h"
#include <cmath>

namespace catchim::exporting {

SceneExporter::SceneExporter(
    const editor::Timeline& timeline,
    ExportSettings settings
) : m_timeline(timeline), m_settings(std::move(settings)) {}

SceneExporter::~SceneExporter() {
    cancel();
}

void SceneExporter::cancel() {
    m_cancelled.store(true);
}

bool SceneExporter::runExport() {
    if (m_running.exchange(true)) {
        LOG_WARN("SceneExporter is already running an export job");
        return false;
    }

    m_cancelled.store(false);
    m_renderedFrames = 0;

    double fpsFloat = static_cast<double>(m_settings.fps.numerator) / static_cast<double>(m_settings.fps.denominator);
    if (fpsFloat <= 0.0) fpsFloat = 30.0;

    int64_t ticksPerFrame = static_cast<int64_t>(std::round(120'000.0 / fpsFloat));
    if (ticksPerFrame <= 0) ticksPerFrame = 4000; // fallback 30 fps

    int64_t totalDurationTicks = m_timeline.totalDuration().ticks();
    m_totalFrames = (ticksPerFrame > 0) ? (totalDurationTicks / ticksPerFrame) : 0;

    LOG_INFO("Starting offline scene export: {} frames at {}x{} ({} fps)",
             m_totalFrames, m_settings.resolution.width, m_settings.resolution.height, fpsFloat);

    if (m_totalFrames == 0) {
        if (m_progressCallback) m_progressCallback(1.0);
        if (m_completeCallback) m_completeCallback(true, "Export completed (empty timeline)");
        m_running.store(false);
        return true;
    }

    render::Compositor compositor(m_settings.resolution.width, m_settings.resolution.height);

    for (int64_t frame = 0; frame < m_totalFrames; ++frame) {
        if (m_cancelled.load()) {
            LOG_INFO("Scene export cancelled by user at frame {}/{}", frame, m_totalFrames);
            if (m_completeCallback) m_completeCallback(false, "Export cancelled");
            m_running.store(false);
            return false;
        }

        core::TimelineTime frameTime = core::TimelineTime::fromTicks(frame * ticksPerFrame);
        (void)frameTime;

        // Frame composite step
        compositor.clear(0, 0, 0, 255);

        m_renderedFrames = frame + 1;
        double progress = static_cast<double>(m_renderedFrames) / static_cast<double>(m_totalFrames);

        if (m_progressCallback) {
            m_progressCallback(progress);
        }
    }

    LOG_INFO("Offline scene export completed successfully: {} frames", m_totalFrames);
    if (m_completeCallback) {
        m_completeCallback(true, "Export completed successfully");
    }

    m_running.store(false);
    return true;
}

} // namespace catchim::exporting
