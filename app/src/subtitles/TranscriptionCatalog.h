#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace catchim::subtitles {

struct LanguageInfo {
    std::string code;
    std::string name;
    std::string nameVi;
};

struct TranscriptionModelInfo {
    std::string id;
    std::string name;
    std::string huggingFaceId;
    std::string description;
};

class TranscriptionCatalog {
public:
    static constexpr const char* DEFAULT_TRANSCRIPTION_MODEL = "whisper-small";
    static constexpr size_t DEFAULT_WORDS_PER_CAPTION = 3;
    static constexpr double MIN_CAPTION_DURATION_SECONDS = 0.8;

    static const std::vector<LanguageInfo>& getSupportedLanguages() noexcept;
    static const LanguageInfo* findLanguageByCode(std::string_view code) noexcept;

    static const std::vector<TranscriptionModelInfo>& getAvailableModels() noexcept;
    static const TranscriptionModelInfo* findModelById(std::string_view id) noexcept;
};

} // namespace catchim::subtitles
