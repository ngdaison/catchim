#include "media/SoundEffectsRegistry.h"
#include <algorithm>

namespace catchim::media {

namespace {

std::string toLower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

} // namespace

SoundEffectsRegistry& SoundEffectsRegistry::instance() {
    static SoundEffectsRegistry s_instance;
    return s_instance;
}

SoundEffectsRegistry::SoundEffectsRegistry() {
    initPresets();
}

void SoundEffectsRegistry::initPresets() {
    auto addSfx = [this](SoundEffectItem item) {
        if (std::find(categories_.begin(), categories_.end(), item.category) == categories_.end()) {
            categories_.push_back(item.category);
        }
        allEffectsList_.push_back(item);
        effects_[item.id] = std::move(item);
    };

    // Whoosh
    addSfx({"whoosh-fast", "Vút nhanh (Fast Whoosh)", "whoosh", core::TimelineTime::fromSeconds(0.5), 48000, 2, "sounds/whoosh/fast.wav", {"whoosh", "fast", "swoosh"}});
    addSfx({"whoosh-deep", "Vút trầm (Deep Whoosh)", "whoosh", core::TimelineTime::fromSeconds(0.8), 48000, 2, "sounds/whoosh/deep.wav", {"whoosh", "deep", "cinematic"}});

    // Impact
    addSfx({"impact-boom", "Va đập lớn (Sub Boom)", "impact", core::TimelineTime::fromSeconds(1.5), 48000, 2, "sounds/impact/boom.wav", {"impact", "boom", "bass"}});
    addSfx({"impact-slam", "Đập mạnh (Door Slam)", "impact", core::TimelineTime::fromSeconds(0.9), 48000, 2, "sounds/impact/slam.wav", {"impact", "slam", "hit"}});

    // Transition
    addSfx({"trans-glitch", "Chuyển cảnh Glitch", "transition", core::TimelineTime::fromSeconds(0.6), 48000, 2, "sounds/transition/glitch.wav", {"glitch", "transition", "digital"}});
    addSfx({"trans-riser", "Tăng dần kịch tính (Riser)", "transition", core::TimelineTime::fromSeconds(2.0), 48000, 2, "sounds/transition/riser.wav", {"riser", "tension", "build"}});

    // UI
    addSfx({"ui-click", "Click chuột nhẹ (Soft Click)", "ui", core::TimelineTime::fromSeconds(0.15), 48000, 2, "sounds/ui/click.wav", {"ui", "click", "tap"}});
    addSfx({"ui-pop", "Tiếng Pop vui tai (Bubble Pop)", "ui", core::TimelineTime::fromSeconds(0.25), 48000, 2, "sounds/ui/pop.wav", {"ui", "pop", "bubble"}});

    // Ambience
    addSfx({"amb-rain", "Mưa rơi êm dịu (Gentle Rain)", "ambience", core::TimelineTime::fromSeconds(5.0), 48000, 2, "sounds/ambience/rain.wav", {"ambience", "rain", "nature"}});
    addSfx({"amb-room", "Không gian phòng tĩnh (Room Tone)", "ambience", core::TimelineTime::fromSeconds(4.0), 48000, 2, "sounds/ambience/room.wav", {"ambience", "room", "silence"}});
}

std::vector<std::string> SoundEffectsRegistry::getCategories() const {
    return categories_;
}

std::vector<SoundEffectItem> SoundEffectsRegistry::getEffectsByCategory(const std::string& category) const {
    std::vector<SoundEffectItem> result;
    for (const auto& item : allEffectsList_) {
        if (item.category == category) {
            result.push_back(item);
        }
    }
    return result;
}

const SoundEffectItem* SoundEffectsRegistry::findEffect(const std::string& id) const noexcept {
    auto it = effects_.find(id);
    if (it != effects_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<SoundEffectItem> SoundEffectsRegistry::searchEffects(const std::string& query) const {
    std::string q = toLower(query);
    if (q.empty()) return allEffectsList_;

    std::vector<SoundEffectItem> result;
    for (const auto& item : allEffectsList_) {
        if (toLower(item.name).find(q) != std::string::npos ||
            toLower(item.category).find(q) != std::string::npos) {
            result.push_back(item);
            continue;
        }
        for (const auto& tag : item.tags) {
            if (toLower(tag).find(q) != std::string::npos) {
                result.push_back(item);
                break;
            }
        }
    }
    return result;
}

editor::Clip SoundEffectsRegistry::createSfxClip(
    const std::string& effectId,
    core::TimelineTime startTime
) const {
    const auto* effect = findEffect(effectId);
    std::string name = effect ? effect->name : "Sound Effect";
    core::TimelineTime dur = effect ? effect->duration : core::TimelineTime::fromSeconds(1.0);

    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Audio,
        name,
        startTime,
        dur
    );

    clip.setParam("sfxId", effectId);
    if (effect) {
        clip.setParam("category", effect->category);
        clip.setParam("sampleRate", effect->sampleRate);
        clip.setParam("channels", effect->channels);
    }
    clip.setParam("volumeDb", 0.0);

    return clip;
}

} // namespace catchim::media
