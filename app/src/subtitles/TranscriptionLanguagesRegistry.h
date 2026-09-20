#pragma once
// Feature 227 -- mirrors web/src/transcription/languages.ts
#include <string>
#include <vector>
#include <optional>

namespace catchim::subtitles {

struct TranscriptionLanguage {
    std::string code;
    std::string name;
    std::string nameVi;
};

class TranscriptionLanguagesRegistry {
public:
    static const std::vector<TranscriptionLanguage>& getAllLanguages();
    static std::optional<TranscriptionLanguage> findByCode(const std::string& code);
    static std::optional<TranscriptionLanguage> findByName(const std::string& name);
    static bool isValidLanguageCode(const std::string& code);
};

} // namespace catchim::subtitles
