#pragma once

#include "ProjectStorage.h"
#include <chrono>
#include <atomic>
#include <functional>

namespace catchim::storage {

class AutosaveManager {
public:
    AutosaveManager(std::chrono::milliseconds interval = std::chrono::milliseconds(3000));

    void markDirty();
    void reset();

    bool shouldSave() const;

    void setInterval(std::chrono::milliseconds interval) { interval_ = interval; }
    std::chrono::milliseconds interval() const noexcept { return interval_; }

private:
    std::chrono::milliseconds interval_{3000};
    std::atomic<bool> isDirty_{false};
    std::chrono::steady_clock::time_point lastChangeTime_;
};

} // namespace catchim::storage
