#include "TranscriptionCatalog.h"

namespace catchim::subtitles {

const std::vector<LanguageInfo>& TranscriptionCatalog::getSupportedLanguages() noexcept {
    static const std::vector<LanguageInfo> kLanguages = {
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
    return kLanguages;
}

const LanguageInfo* TranscriptionCatalog::findLanguageByCode(std::string_view code) noexcept {
    const auto& languages = getSupportedLanguages();
    for (const auto& lang : languages) {
        if (lang.code == code) {
            return &lang;
        }
    }
    return nullptr;
}

const std::vector<TranscriptionModelInfo>& TranscriptionCatalog::getAvailableModels() noexcept {
    static const std::vector<TranscriptionModelInfo> kModels = {
        {
            "whisper-tiny",
            "Tiny",
            "onnx-community/whisper-tiny",
            "Fastest, lower accuracy"
        },
        {
            "whisper-small",
            "Small",
            "onnx-community/whisper-small",
            "Good balance of speed and accuracy"
        },
        {
            "whisper-medium",
            "Medium",
            "onnx-community/whisper-medium",
            "Higher accuracy, slower"
        },
        {
            "whisper-large-v3-turbo",
            "Large v3 Turbo",
            "onnx-community/whisper-large-v3-turbo",
            "Best accuracy, requires WebGPU for good performance"
        }
    };
    return kModels;
}

const TranscriptionModelInfo* TranscriptionCatalog::findModelById(std::string_view id) noexcept {
    const auto& models = getAvailableModels();
    for (const auto& model : models) {
        if (model.id == id) {
            return &model;
        }
    }
    return nullptr;
}

} // namespace catchim::subtitles
