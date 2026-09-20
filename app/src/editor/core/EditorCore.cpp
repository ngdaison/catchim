#include "EditorCore.h"

namespace catchim::editor {

std::unique_ptr<EditorCore> EditorCore::instance_{nullptr};

EditorCore& EditorCore::getInstance() {
    if (!instance_) {
        instance_ = std::make_unique<EditorCore>();
    }
    return *instance_;
}

void EditorCore::reset() {
    ClipboardManager::instance().clear();
    instance_.reset();
}

EditorCore::EditorCore() {
    // Create initial default project
    projectManager_.createNewProject("Default Project");
    timelineManager_ = std::make_unique<TimelineManager>(*projectManager_.getActive());

    // Bind playback timeline scope
    bindTimelineScope();

    // Setup save action
    saveManager_.setSaveAction([this]() {
        if (projectManager_.getActive()) {
            projectManager_.setDirty(false);
        }
    });
    saveManager_.start();
}

void EditorCore::bindTimelineScope() {
    timelineManager_->subscribe([this]() {
        const auto totalDuration = timelineManager_->getTotalDuration();
        playbackManager_.reconcileTimelineScope(totalDuration);
    });
}

void EditorCore::registerReactor(std::function<void()> reactor) {
    if (reactor) {
        reactors_.push_back(std::move(reactor));
    }
}

void EditorCore::runReactors() {
    for (const auto& reactor : reactors_) {
        if (reactor) {
            reactor();
        }
    }
}

} // namespace catchim::editor
