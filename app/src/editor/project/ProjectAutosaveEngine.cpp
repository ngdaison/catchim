#include "editor/project/ProjectAutosaveEngine.h"

namespace catchim::editor {

ProjectAutosaveEngine::ProjectAutosaveEngine(int64_t debounceIntervalMs)
    : debounceIntervalMs_(debounceIntervalMs)
{
}

void ProjectAutosaveEngine::pause() noexcept {
    isPaused_ = true;
}

void ProjectAutosaveEngine::resume() noexcept {
    isPaused_ = false;
    if (hasPendingSave_) {
        notifyStatus();
    }
}

void ProjectAutosaveEngine::markDirty(bool force) noexcept {
    if (isPaused_ && !force) {
        return;
    }
    hasPendingSave_ = true;
    notifyStatus();
}

bool ProjectAutosaveEngine::canSaveNow(
    bool hasActiveProject,
    bool isProjectLoading,
    bool isMigrating
) const noexcept {
    if (!hasActiveProject || isProjectLoading || isMigrating || isSaving_) {
        return false;
    }
    return true;
}

bool ProjectAutosaveEngine::flush(
    bool hasActiveProject,
    bool isProjectLoading,
    bool isMigrating
) {
    if (!canSaveNow(hasActiveProject, isProjectLoading, isMigrating)) {
        return false;
    }

    if (!hasPendingSave_) {
        return true;
    }

    beginSave();
    bool success = false;
    if (saveExecutor_) {
        success = saveExecutor_();
    } else {
        success = true;
    }
    endSave(success);
    return success;
}

void ProjectAutosaveEngine::beginSave() noexcept {
    isSaving_ = true;
    hasPendingSave_ = false;
    notifyStatus();
}

void ProjectAutosaveEngine::endSave(bool success) noexcept {
    isSaving_ = false;
    if (!success) {
        // Re-queue pending save if save failed
        hasPendingSave_ = true;
    }
    notifyStatus();
}

void ProjectAutosaveEngine::notifyStatus() noexcept {
    if (statusListener_) {
        statusListener_(isDirty());
    }
}

} // namespace catchim::editor
