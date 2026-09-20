#pragma once

#include <functional>
#include <chrono>
#include <cstdint>

namespace catchim::editor {

class ProjectAutosaveEngine {
public:
    using SaveExecutor = std::function<bool()>;
    using StatusListener = std::function<void(bool isDirty)>;

    explicit ProjectAutosaveEngine(int64_t debounceIntervalMs = 800);

    int64_t debounceIntervalMs() const noexcept { return debounceIntervalMs_; }
    void setDebounceIntervalMs(int64_t ms) noexcept { debounceIntervalMs_ = ms; }

    void setSaveExecutor(SaveExecutor executor) { saveExecutor_ = std::move(executor); }
    void setStatusListener(StatusListener listener) { statusListener_ = std::move(listener); }

    void pause() noexcept;
    void resume() noexcept;
    bool isPaused() const noexcept { return isPaused_; }

    void markDirty(bool force = false) noexcept;
    bool isDirty() const noexcept { return hasPendingSave_ || isSaving_; }
    bool isSaving() const noexcept { return isSaving_; }
    bool hasPendingSave() const noexcept { return hasPendingSave_; }

    bool canSaveNow(
        bool hasActiveProject,
        bool isProjectLoading,
        bool isMigrating
    ) const noexcept;

    // Synchronous flush - executes save immediately if pending or forced
    bool flush(
        bool hasActiveProject = true,
        bool isProjectLoading = false,
        bool isMigrating = false
    );

    // Manual lifecycle control if caller manages the save worker
    void beginSave() noexcept;
    void endSave(bool success) noexcept;

private:
    void notifyStatus() noexcept;

    int64_t debounceIntervalMs_{800};
    bool isPaused_{false};
    bool isSaving_{false};
    bool hasPendingSave_{false};

    SaveExecutor saveExecutor_;
    StatusListener statusListener_;
};

} // namespace catchim::editor
