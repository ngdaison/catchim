#include "SavedSoundsStore.h"

namespace catchim::media {

bool SavedSoundsStore::isSoundSaved(int64_t soundId) const noexcept {
    for (const auto& sound : savedSounds_) {
        if (sound.id == soundId) {
            return true;
        }
    }
    return false;
}

void SavedSoundsStore::saveSoundEffect(const SoundEffect& sound) {
    if (isSoundSaved(sound.id)) {
        return;
    }

    savedSounds_.push_back(SavedSound{
        .id = sound.id,
        .name = sound.name,
        .username = sound.username,
        .previewUrl = sound.previewUrl,
        .downloadUrl = sound.downloadUrl,
        .duration = sound.duration,
        .tags = sound.tags,
        .license = sound.license,
        .savedAt = "2026-09-20T00:00:00Z"
    });
    notify();
}

bool SavedSoundsStore::removeSavedSound(int64_t soundId) noexcept {
    auto it = std::remove_if(savedSounds_.begin(), savedSounds_.end(), [&](const SavedSound& s) {
        return s.id == soundId;
    });

    if (it != savedSounds_.end()) {
        savedSounds_.erase(it, savedSounds_.end());
        notify();
        return true;
    }
    return false;
}

void SavedSoundsStore::toggleSavedSound(const SoundEffect& sound) {
    if (isSoundSaved(sound.id)) {
        removeSavedSound(sound.id);
    } else {
        saveSoundEffect(sound);
    }
}

void SavedSoundsStore::clearSavedSounds() noexcept {
    if (!savedSounds_.empty()) {
        savedSounds_.clear();
        notify();
    }
}

void SavedSoundsStore::subscribe(ChangeListener listener) {
    listeners_.push_back(std::move(listener));
}

void SavedSoundsStore::notify() {
    for (const auto& l : listeners_) {
        if (l) l();
    }
}

} // namespace catchim::media
