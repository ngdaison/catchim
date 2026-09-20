#pragma once

#include "editor/project/ProjectManager.h"
#include "editor/timeline/TimelineManager.h"
#include "editor/playback/PlaybackManager.h"
#include "render/RendererManager.h"
#include "editor/project/SaveManager.h"
#include "media/MediaManager.h"
#include "audio/AudioManager.h"
#include "editor/selection/EditorSelection.h"
#include "editor/clipboard/ClipboardManager.h"
#include "editor/history/CommandHistory.h"
#include "editor/diagnostics/DiagnosticsRegistry.h"
#include <memory>
#include <vector>
#include <functional>

namespace catchim::editor {

class EditorCore {
public:
    static EditorCore& getInstance();
    static void reset();

    EditorCore();
    ~EditorCore() = default;

    // Managers
    ProjectManager& project() noexcept { return projectManager_; }
    const ProjectManager& project() const noexcept { return projectManager_; }

    TimelineManager& timeline() noexcept { return *timelineManager_; }
    const TimelineManager& timeline() const noexcept { return *timelineManager_; }

    PlaybackManager& playback() noexcept { return playbackManager_; }
    const PlaybackManager& playback() const noexcept { return playbackManager_; }

    render::RendererManager& renderer() noexcept { return rendererManager_; }
    const render::RendererManager& renderer() const noexcept { return rendererManager_; }

    SaveManager& save() noexcept { return saveManager_; }
    const SaveManager& save() const noexcept { return saveManager_; }

    media::MediaManager& media() noexcept { return mediaManager_; }
    const media::MediaManager& media() const noexcept { return mediaManager_; }

    audio::AudioManager& audio() noexcept { return audioManager_; }
    const audio::AudioManager& audio() const noexcept { return audioManager_; }

    EditorSelection& selection() noexcept { return selection_; }
    const EditorSelection& selection() const noexcept { return selection_; }

    ClipboardManager& clipboard() noexcept { return ClipboardManager::instance(); }
    const ClipboardManager& clipboard() const noexcept { return ClipboardManager::instance(); }

    CommandHistory& command() noexcept { return commandHistory_; }
    const CommandHistory& command() const noexcept { return commandHistory_; }

    DiagnosticsRegistry& diagnostics() noexcept { return DiagnosticsRegistry::instance(); }
    const DiagnosticsRegistry& diagnostics() const noexcept { return DiagnosticsRegistry::instance(); }

    // Reactors & Bindings
    void registerReactor(std::function<void()> reactor);
    void runReactors();
    void bindTimelineScope();

private:
    static std::unique_ptr<EditorCore> instance_;

    ProjectManager projectManager_;
    std::unique_ptr<TimelineManager> timelineManager_;
    PlaybackManager playbackManager_;
    render::RendererManager rendererManager_;
    SaveManager saveManager_;
    media::MediaManager mediaManager_;
    audio::AudioManager audioManager_;
    EditorSelection selection_;
    CommandHistory commandHistory_;
    std::vector<std::function<void()>> reactors_;
};

} // namespace catchim::editor
