#include "media/PresetManager.h"
#include <algorithm>

namespace catchim::media {

PresetManager& PresetManager::instance() {
    static PresetManager s_instance;
    return s_instance;
}

PresetManager::PresetManager() {
    initDefaults();
}

void PresetManager::initDefaults() {
    // 1. Aspect Ratios
    aspectRatios_ = {
        {"16:9", "Widescreen (16:9)", 1920, 1080},
        {"9:16", "Vertical / TikTok (9:16)", 1080, 1920},
        {"1:1", "Square (1:1)", 1080, 1080},
        {"4:5", "Portrait (4:5)", 1080, 1350},
        {"21:9", "Ultrawide (21:9)", 2560, 1080}
    };

    // 2. Text Presets
    textPresets_ = {
        {"text-default", "Mặc định", "Văn bản tiêu chuẩn", 36.0, "Inter", "#FFFFFF", 0.0, 0.0, false, false},
        {"text-lowerthird", "Lower Third", "Tên diễn giả / Tiêu đề phụ", 28.0, "Inter", "#E0E0E0", -300.0, 250.0, false, false},
        {"text-boldcenter", "Tiêu đề lớn", "TIÊU ĐỀ CHÍNH", 64.0, "Inter", "#FFFF00", 0.0, 0.0, true, false},
        {"text-subtitle", "Phụ đề nét viền", "Nội dung lời thoại", 32.0, "Inter", "#FFFFFF", 0.0, 350.0, false, false}
    };

    // 3. Transition Presets
    transitionPresets_ = {
        {"trans-crossfade", "Hòa tan (Crossfade)", "crossfade", core::TimelineTime::fromSeconds(1.0)},
        {"trans-fadeblack", "Mờ dần sang đen", "fade-to-black", core::TimelineTime::fromSeconds(0.8)},
        {"trans-slideleft", "Trượt sang trái", "slide-left", core::TimelineTime::fromSeconds(0.5)},
        {"trans-zoomin", "Thu phóng (Zoom In)", "zoom-in", core::TimelineTime::fromSeconds(0.6)}
    };

    // 4. Audio Master Presets
    audioPresets_ = {
        {"audio-standard", "Tiêu chuẩn Video (-1 dB)", -1.0, 0.95, 0.1},
        {"audio-podcast", "Podcast & Giọng nói (-1.5 dB)", -1.5, 0.90, 0.15},
        {"audio-loud", "Mạng xã hội / Reels (0 dB)", 0.0, 0.98, 0.05}
    };
}

const AspectRatioPreset* PresetManager::findAspectRatio(const std::string& id) const noexcept {
    for (const auto& p : aspectRatios_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

const TextTitlePreset* PresetManager::findTextPreset(const std::string& id) const noexcept {
    for (const auto& p : textPresets_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

editor::Clip PresetManager::createTextClipFromPreset(
    const std::string& presetId,
    const std::string& customText,
    core::TimelineTime startTime,
    core::TimelineTime duration
) const {
    const auto* preset = findTextPreset(presetId);
    std::string text = customText.empty() ? (preset ? preset->defaultText : "Text") : customText;
    std::string name = preset ? preset->name : "Text Clip";

    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Text,
        name,
        startTime,
        duration
    );

    clip.setParam("text", text);
    if (preset) {
        clip.setParam("presetId", preset->id);
        clip.setParam("fontSize", preset->fontSize);
        clip.setParam("fontFamily", preset->fontFamily);
        clip.setParam("fontColor", preset->fontColor);
        clip.setParam("transform.positionX", preset->posX);
        clip.setParam("transform.positionY", preset->posY);
        clip.setParam("bold", preset->bold);
        clip.setParam("italic", preset->italic);
    } else {
        clip.setParam("fontSize", 36.0);
        clip.setParam("fontColor", std::string("#FFFFFF"));
    }

    return clip;
}

const TransitionPreset* PresetManager::findTransitionPreset(const std::string& id) const noexcept {
    for (const auto& p : transitionPresets_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

const AudioMasterPreset* PresetManager::findAudioPreset(const std::string& id) const noexcept {
    for (const auto& p : audioPresets_) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

} // namespace catchim::media
