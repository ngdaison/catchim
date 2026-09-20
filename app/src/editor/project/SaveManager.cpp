#include "SaveManager.h"

namespace catchim::editor {

SaveManager::SaveManager(SaveAction saveAction, int debounceMs)
    : saveAction_(std::move(saveAction)), debounceMs_(debounceMs) {}

void SaveManager::start() noexcept {
    isRunning_ = true;
}

void SaveManager::stop() noexcept {
    isRunning_ = false;
    hasPendingSave_ = false;
}

void SaveManager::pause() noexcept {
    isPaused_ = true;
}

void SaveManager::resume() {
    isPaused_ = false;
    if (hasPendingSave_) {
        triggerSaveNow();
    }
}

void SaveManager::markDirty(bool force) {
    if (isPaused_ && !force) {
        return;
    }
    hasPendingSave_ = true;
    if (isRunning_ && !isPaused_) {
        triggerSaveNow();
    }
}

void SaveManager::flush() {
    hasPendingSave_ = true;
    triggerSaveNow();
}

void SaveManager::triggerSaveNow() {
    if (isSaving_ || !hasPendingSave_) {
        return;
    }

    isSaving_ = true;
    hasPendingSave_ = false;

    if (saveAction_) {
        saveAction_();
    }

    isSaving_ = false;
}

} // namespace catchim::editor
