#include "AutosaveManager.h"

namespace catchim::storage {

AutosaveManager::AutosaveManager(std::chrono::milliseconds interval)
    : interval_(interval)
    , lastChangeTime_(std::chrono::steady_clock::now())
{
}

void AutosaveManager::markDirty() {
    isDirty_ = true;
    lastChangeTime_ = std::chrono::steady_clock::now();
}

void AutosaveManager::reset() {
    isDirty_ = false;
}

bool AutosaveManager::shouldSave() const {
    if (!isDirty_) return false;
    auto now = std::chrono::steady_clock::now();
    return (now - lastChangeTime_) >= interval_;
}

} // namespace catchim::storage
