#pragma once

#include <string>
#include <vector>
#include <optional>

namespace catchim::media {

struct TtsVoice {
    std::string id;
    std::string name;
    std::string language; // "vi-VN", "en-US", etc.
    std::string gender;   // "female", "male", "child", "special"
    std::vector<std::string> categories; // "all", "vietnamese", "english", "trending", "story", "fun"
    std::string description;
    std::string avatarGradient;
    std::string avatarIcon;
    std::string previewSampleText;
    std::string engine;   // "edge", "google", "web-speech", "gpt-sovits"
    std::string edgeVoiceName;
    std::string langCode; // "vi", "en"
    double pitchFactor{1.0};
    std::string audioEffect{"none"};
};

class TtsVoiceRegistry {
public:
    static const std::vector<TtsVoice>& getAllVoices() noexcept;
    static const TtsVoice* findVoiceById(const std::string& id) noexcept;
    static std::vector<const TtsVoice*> getVoicesByLanguage(const std::string& language);
    static std::vector<const TtsVoice*> getVoicesByCategory(const std::string& category);
    static std::vector<const TtsVoice*> getVoicesByGender(const std::string& gender);
};

} // namespace catchim::media
