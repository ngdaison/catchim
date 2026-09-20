#pragma once

#include "audio/AudioBuffer.h"
#include <string>
#include <vector>
#include <cstdint>

namespace catchim::audio {

struct TtsVoice {
    std::string id;
    std::string name;
    std::string language; // e.g. "vi-VN", "en-US"
    std::string gender;   // "female", "male", "special"
    std::string description;
    std::string previewSampleText;
    double rateMultiplier{1.0};
    double pitchFactor{1.0};
};

class TtsEngine {
public:
    static TtsEngine& instance();

    TtsEngine();

    const std::vector<TtsVoice>& voices() const noexcept { return voices_; }
    const TtsVoice* findVoice(const std::string& id) const noexcept;
    std::vector<TtsVoice> findVoicesByLanguage(const std::string& langPrefix) const;

    static double estimateSpeechDuration(
        const std::string& text,
        double rateMultiplier = 1.0,
        const std::string& lang = "vi"
    );

    static std::vector<uint8_t> encodePcm16Wav(const AudioBuffer& buffer);

private:
    void initVoices();

    std::vector<TtsVoice> voices_;
};

} // namespace catchim::audio
