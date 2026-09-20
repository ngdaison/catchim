#include "TtsVoiceService.h"

#include <algorithm>

namespace catchim::media {

const std::vector<TtsVoice>& TtsVoiceService::getAllVoices() {
    return TtsVoiceRegistry::getAllVoices();
}

std::vector<TtsVoice> TtsVoiceService::filterByCategory(const std::string& category) {
    if (category.empty() || category == "all") {
        return getAllVoices();
    }

    std::vector<TtsVoice> result;
    for (const auto& voice : getAllVoices()) {
        if (std::find(voice.categories.begin(), voice.categories.end(), category) != voice.categories.end()) {
            result.push_back(voice);
        }
    }
    return result;
}

std::vector<TtsVoice> TtsVoiceService::filterByLanguage(const std::string& langCode) {
    std::vector<TtsVoice> result;
    for (const auto& voice : getAllVoices()) {
        if (voice.language.rfind(langCode, 0) == 0 || voice.langCode == langCode) {
            result.push_back(voice);
        }
    }
    return result;
}

std::vector<TtsVoice> TtsVoiceService::filterByGender(const std::string& gender) {
    std::vector<TtsVoice> result;
    for (const auto& voice : getAllVoices()) {
        if (voice.gender == gender) {
            result.push_back(voice);
        }
    }
    return result;
}

std::optional<TtsVoice> TtsVoiceService::findVoiceById(const std::string& id) {
    const auto* voicePtr = TtsVoiceRegistry::findVoiceById(id);
    if (voicePtr) {
        return *voicePtr;
    }
    return std::nullopt;
}

bool TtsVoiceService::validateOptions(const TtsSynthesisOptions& options) noexcept {
    if (options.text.empty()) {
        return false;
    }
    return validateOptions(options.speed, options.pitch, options.volume);
}

bool TtsVoiceService::validateOptions(double speed, double pitch, double volume) noexcept {
    if (speed < 0.5 || speed > 2.0) {
        return false;
    }
    if (pitch < -50.0 || pitch > 50.0) {
        return false;
    }
    if (volume < 0.0 || volume > 1.0) {
        return false;
    }
    return true;
}

} // namespace catchim::media
