#pragma once

#include <functional>
#include <chrono>

namespace catchim::editor {

class SaveManager {
public:
    using SaveAction = std::function<void()>;

    explicit SaveManager(SaveAction saveAction = nullptr, int debounceMs = 800);

    void setSaveAction(SaveAction action) noexcept { saveAction_ = std::move(action); }
    int debounceMs() const noexcept { return debounceMs_; }
    void setDebounceMs(int ms) noexcept { debounceMs_ = ms; }

    void start() noexcept;
    void stop() noexcept;
    void pause() noexcept;
    void resume();

    void markDirty(bool force = false);
    void flush();

    bool isDirty() const noexcept { return hasPendingSave_ || isSaving_; }
    bool isPaused() const noexcept { return isPaused_; }
    bool isSaving() const noexcept { return isSaving_; }

private:
    void triggerSaveNow();

    SaveAction saveAction_;
    int debounceMs_{800};
    bool isRunning_{false};
    bool isPaused_{false};
    bool isSaving_{false};
    bool hasPendingSave_{false};
};

} // namespace catchim::editor
