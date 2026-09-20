#pragma once

#include <optional>
#include <utility>

namespace catchim::editor {

/**
 * Generic Preview Tracker for interactive live previews before committing commands.
 * Mirrors web/src/commands/preview-tracker.ts.
 */
template <typename T>
class PreviewTracker {
public:
    PreviewTracker() = default;

    void begin(const T& state) {
        if (!snapshot_.has_value()) {
            snapshot_ = state;
        }
    }

    void begin(T&& state) {
        if (!snapshot_.has_value()) {
            snapshot_ = std::move(state);
        }
    }

    bool isActive() const noexcept {
        return snapshot_.has_value();
    }

    const T* getSnapshot() const noexcept {
        return snapshot_.has_value() ? &snapshot_.value() : nullptr;
    }

    std::optional<T> end() {
        std::optional<T> snapshot = std::move(snapshot_);
        snapshot_.reset();
        return snapshot;
    }

    void cancel() noexcept {
        snapshot_.reset();
    }

private:
    std::optional<T> snapshot_;
};

} // namespace catchim::editor
