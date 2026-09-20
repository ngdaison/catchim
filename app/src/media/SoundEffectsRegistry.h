#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace catchim::media {

struct SoundEffectItem {
    std::string id;
    std::string name;
    std::string category; // "whoosh", "impact", "transition", "ui", "ambience"
    core::TimelineTime duration;
    int32_t sampleRate{48000};
    int32_t channels{2};
    std::string assetPath;
    std::vector<std::string> tags;
};

class SoundEffectsRegistry {
public:
    static SoundEffectsRegistry& instance();

    SoundEffectsRegistry();

    const std::vector<SoundEffectItem>& allEffects() const noexcept { return allEffectsList_; }
    std::vector<std::string> getCategories() const;
    std::vector<SoundEffectItem> getEffectsByCategory(const std::string& category) const;
    const SoundEffectItem* findEffect(const std::string& id) const noexcept;
    std::vector<SoundEffectItem> searchEffects(const std::string& query) const;

    editor::Clip createSfxClip(
        const std::string& effectId,
        core::TimelineTime startTime
    ) const;

    size_t totalCount() const noexcept { return effects_.size(); }

private:
    void initPresets();

    std::unordered_map<std::string, SoundEffectItem> effects_;
    std::vector<SoundEffectItem> allEffectsList_;
    std::vector<std::string> categories_;
};

} // namespace catchim::media
