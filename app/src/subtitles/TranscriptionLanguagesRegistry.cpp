// Feature 227 -- mirrors web/src/transcription/languages.ts
#include "subtitles/TranscriptionLanguagesRegistry.h"
#include <algorithm>

namespace catchim::subtitles {

static const std::vector<TranscriptionLanguage> s_languages = {
    {"en", "English", "Tiếng Anh"},
    {"es", "Spanish", "Tiếng Tây Ban Nha"},
    {"it", "Italian", "Tiếng Ý"},
    {"fr", "French", "Tiếng Pháp"},
    {"de", "German", "Tiếng Đức"},
    {"pt", "Portuguese", "Tiếng Bồ Đào Nha"},
    {"ru", "Russian", "Tiếng Nga"},
    {"ja", "Japanese", "Tiếng Nhật"},
    {"vi", "Vietnamese", "Tiếng Việt"},
    {"zh", "Chinese", "Tiếng Trung"}
};

const std::vector<TranscriptionLanguage>& TranscriptionLanguagesRegistry::getAllLanguages() {
    return s_languages;
}

std::optional<TranscriptionLanguage> TranscriptionLanguagesRegistry::findByCode(const std::string& code) {
    for (const auto& lang : s_languages) {
        if (lang.code == code) {
            return lang;
        }
    }
    return std::nullopt;
}

std::optional<TranscriptionLanguage> TranscriptionLanguagesRegistry::findByName(const std::string& name) {
    for (const auto& lang : s_languages) {
        if (lang.name == name || lang.nameVi == name) {
            return lang;
        }
    }
    return std::nullopt;
}

bool TranscriptionLanguagesRegistry::isValidLanguageCode(const std::string& code) {
    return findByCode(code).has_value();
}

} // namespace catchim::subtitles
