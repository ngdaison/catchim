#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <optional>

namespace catchim::media {

struct SoundEffect {
    int64_t id{0};
    std::string name;
    std::string description;
    std::string url;
    std::string previewUrl;
    std::string downloadUrl;
    double duration{0.0};
    int64_t filesize{0};
    std::string type{"wav"};
    int channels{2};
    int samplerate{44100};
    std::string username;
    std::vector<std::string> tags;
    std::string license;
    int downloads{0};
    double rating{0.0};
    int ratingCount{0};
};

struct SavedSound {
    int64_t id{0};
    std::string name;
    std::string username;
    std::string previewUrl;
    std::string downloadUrl;
    double duration{0.0};
    std::vector<std::string> tags;
    std::string license;
    std::string savedAt;

    bool operator==(const SavedSound& other) const noexcept = default;
};

class SavedSoundsStore {
public:
    using ChangeListener = std::function<void()>;

    SavedSoundsStore() = default;

    const std::vector<SavedSound>& getSavedSounds() const noexcept { return savedSounds_; }
    bool isSoundSaved(int64_t soundId) const noexcept;

    void saveSoundEffect(const SoundEffect& sound);
    bool removeSavedSound(int64_t soundId) noexcept;
    void toggleSavedSound(const SoundEffect& sound);
    void clearSavedSounds() noexcept;

    size_t count() const noexcept { return savedSounds_.size(); }

    void subscribe(ChangeListener listener);

private:
    void notify();

    std::vector<SavedSound> savedSounds_;
    std::vector<ChangeListener> listeners_;
};

} // namespace catchim::media
