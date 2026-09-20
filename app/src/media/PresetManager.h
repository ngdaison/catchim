#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <vector>

namespace catchim::media {

struct AspectRatioPreset {
    std::string id;
    std::string name;
    int width{1920};
    int height{1080};

    double ratio() const noexcept {
        return height > 0 ? static_cast<double>(width) / height : (16.0 / 9.0);
    }
};

struct TextTitlePreset {
    std::string id;
    std::string name;
    std::string defaultText;
    double fontSize{36.0};
    std::string fontFamily{"Inter"};
    std::string fontColor{"#FFFFFF"};
    double posX{0.0};
    double posY{0.0};
    bool bold{false};
    bool italic{false};
};

struct TransitionPreset {
    std::string id;
    std::string name;
    std::string transitionType{"crossfade"};
    core::TimelineTime defaultDuration{core::TimelineTime::fromSeconds(1.0)};
};

struct AudioMasterPreset {
    std::string id;
    std::string name;
    double targetPeakDb{-1.0};
    double limiterHeadroom{0.95};
    double releaseTimeSec{0.1};
};

class PresetManager {
public:
    static PresetManager& instance();

    PresetManager();

    // Aspect Ratios
    const std::vector<AspectRatioPreset>& aspectRatios() const noexcept { return aspectRatios_; }
    const AspectRatioPreset* findAspectRatio(const std::string& id) const noexcept;

    // Text Presets
    const std::vector<TextTitlePreset>& textPresets() const noexcept { return textPresets_; }
    const TextTitlePreset* findTextPreset(const std::string& id) const noexcept;
    editor::Clip createTextClipFromPreset(
        const std::string& presetId,
        const std::string& customText,
        core::TimelineTime startTime,
        core::TimelineTime duration = core::TimelineTime::fromSeconds(3.0)
    ) const;

    // Transition Presets
    const std::vector<TransitionPreset>& transitionPresets() const noexcept { return transitionPresets_; }
    const TransitionPreset* findTransitionPreset(const std::string& id) const noexcept;

    // Audio Master Presets
    const std::vector<AudioMasterPreset>& audioPresets() const noexcept { return audioPresets_; }
    const AudioMasterPreset* findAudioPreset(const std::string& id) const noexcept;

private:
    void initDefaults();

    std::vector<AspectRatioPreset> aspectRatios_;
    std::vector<TextTitlePreset> textPresets_;
    std::vector<TransitionPreset> transitionPresets_;
    std::vector<AudioMasterPreset> audioPresets_;
};

} // namespace catchim::media
