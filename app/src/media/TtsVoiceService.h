#pragma once

#include "TtsVoiceRegistry.h"
#include <optional>
#include <string>
#include <vector>

namespace catchim::media {

struct TtsSynthesisOptions {
    std::string text;
    std::string voiceId;
    double speed{1.0};  // 0.5 to 2.0
    double pitch{0.0};  // -50 to 50
    double volume{1.0}; // 0.0 to 1.0
};

class TtsVoiceService {
public:
    static const std::vector<TtsVoice>& getAllVoices();
    static std::vector<TtsVoice> filterByCategory(const std::string& category);
    static std::vector<TtsVoice> filterByLanguage(const std::string& langCode);
    static std::vector<TtsVoice> filterByGender(const std::string& gender);
    static std::optional<TtsVoice> findVoiceById(const std::string& id);

    static bool validateOptions(const TtsSynthesisOptions& options) noexcept;
    static bool validateOptions(double speed, double pitch, double volume) noexcept;
};

} // namespace catchim::media
