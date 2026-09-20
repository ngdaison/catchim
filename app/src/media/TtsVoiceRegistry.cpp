#include "TtsVoiceRegistry.h"
#include <algorithm>

namespace catchim::media {

const std::vector<TtsVoice>& TtsVoiceRegistry::getAllVoices() noexcept {
    static const std::vector<TtsVoice> voices = {
        TtsVoice{
            .id = "vi-custom-gpt-sovits",
            .name = "Giọng Clone AI (GPT-SoVITS)",
            .language = "vi-VN",
            .gender = "special",
            .categories = {"vietnamese", "trending"},
            .description = "Giọng nhân bản từ model apps/web/models/voice/ (.pth & .ckpt)",
            .avatarGradient = "from-purple-600 via-indigo-600 to-pink-500",
            .avatarIcon = "✨",
            .previewSampleText = "Xin chào các bạn, đây là giọng đọc nhân bản AI từ mô hình GPT-SoVITS của bạn.",
            .engine = "gpt-sovits",
            .edgeVoiceName = "",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-female-sweet",
            .name = "Hoài My (Nữ miền Bắc)",
            .language = "vi-VN",
            .gender = "female",
            .categories = {"vietnamese", "trending"},
            .description = "Giọng nữ miền Bắc ngọt ngào, trong trẻo, truyền cảm chuẩn TikTok",
            .avatarGradient = "from-pink-500 via-rose-500 to-red-400",
            .avatarIcon = "🌸",
            .previewSampleText = "Xin chào! Mình là Hoài My, giọng nữ Bắc ngọt ngào và tự nhiên.",
            .engine = "edge",
            .edgeVoiceName = "vi-VN-HoaiMyNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-male-warm",
            .name = "Nam Minh (Nam miền Bắc)",
            .language = "vi-VN",
            .gender = "male",
            .categories = {"vietnamese", "trending", "story"},
            .description = "Giọng nam miền Bắc trầm ấm, đĩnh đạc, phát thanh viên chuyên nghiệp",
            .avatarGradient = "from-blue-600 via-indigo-500 to-cyan-400",
            .avatarIcon = "🎙️",
            .previewSampleText = "Chào bạn, đây là giọng nam trầm ấm Nam Minh, rất vui được đồng hành cùng bạn.",
            .engine = "edge",
            .edgeVoiceName = "vi-VN-NamMinhNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-female-google",
            .name = "Chị Google (Việt Nam)",
            .language = "vi-VN",
            .gender = "female",
            .categories = {"vietnamese", "trending", "fun"},
            .description = "Giọng đọc Google nguyên bản quen thuộc, kinh điển trên mạng xã hội",
            .avatarGradient = "from-amber-400 via-orange-500 to-yellow-500",
            .avatarIcon = "📢",
            .previewSampleText = "Xin chào các bạn! Tôi là chị Google, giọng đọc huyền thoại của các video viral trên mạng xã hội.",
            .engine = "google",
            .edgeVoiceName = "",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-intl-andrew",
            .name = "Andrew (Đa ngôn ngữ Nam)",
            .language = "vi-VN",
            .gender = "male",
            .categories = {"vietnamese", "trending"},
            .description = "Giọng nam AI đa ngôn ngữ Microsoft đọc tiếng Việt hiện đại, trẻ trung",
            .avatarGradient = "from-cyan-500 via-teal-500 to-emerald-500",
            .avatarIcon = "✈️",
            .previewSampleText = "Xin chào, tôi là Andrew, giọng đọc AI đa ngôn ngữ với phong cách trẻ trung, hiện đại.",
            .engine = "edge",
            .edgeVoiceName = "en-US-AndrewMultilingualNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-intl-ava",
            .name = "Ava (Đa ngôn ngữ Nữ)",
            .language = "vi-VN",
            .gender = "female",
            .categories = {"vietnamese", "trending"},
            .description = "Giọng nữ AI đa ngôn ngữ Microsoft đọc tiếng Việt tươi sáng, năng động",
            .avatarGradient = "from-purple-500 via-violet-500 to-indigo-500",
            .avatarIcon = "💄",
            .previewSampleText = "Xin chào, mình là Ava, giọng đọc phong cách sống tươi tắn và năng động.",
            .engine = "edge",
            .edgeVoiceName = "en-US-AvaMultilingualNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-intl-brian",
            .name = "Brian (Đa ngôn ngữ Trầm)",
            .language = "vi-VN",
            .gender = "male",
            .categories = {"vietnamese", "story"},
            .description = "Giọng nam AI đa ngôn ngữ Microsoft đọc tiếng Việt trầm ổn, cuốn hút",
            .avatarGradient = "from-stone-700 via-neutral-800 to-zinc-900",
            .avatarIcon = "🎬",
            .previewSampleText = "Xin chào các bạn, tôi là Brian, giọng đọc trầm ấm phù hợp cho những câu chuyện sâu lắng.",
            .engine = "edge",
            .edgeVoiceName = "en-US-BrianMultilingualNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-intl-emma",
            .name = "Emma (Đa ngôn ngữ Nhã nhặn)",
            .language = "vi-VN",
            .gender = "female",
            .categories = {"vietnamese", "story"},
            .description = "Giọng nữ AI đa ngôn ngữ Microsoft đọc tiếng Việt từ tốn, thanh lịch",
            .avatarGradient = "from-rose-400 via-pink-400 to-amber-300",
            .avatarIcon = "☕",
            .previewSampleText = "Xin chào quý vị và các bạn, tôi là Emma, rất vui được mang đến giọng đọc thanh lịch này.",
            .engine = "edge",
            .edgeVoiceName = "en-US-EmmaMultilingualNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        },
        TtsVoice{
            .id = "vi-intl-william",
            .name = "William (Đa ngôn ngữ Quốc tế)",
            .language = "vi-VN",
            .gender = "male",
            .categories = {"vietnamese"},
            .description = "Giọng nam AI đa ngôn ngữ Microsoft phát âm tiếng Việt chuẩn xác",
            .avatarGradient = "from-blue-700 via-sky-600 to-cyan-500",
            .avatarIcon = "🌐",
            .previewSampleText = "Xin chào, tôi là William, giọng đọc đa ngôn ngữ chuẩn mực và rõ ràng.",
            .engine = "edge",
            .edgeVoiceName = "en-AU-WilliamMultilingualNeural",
            .langCode = "vi",
            .pitchFactor = 1.0,
            .audioEffect = "none"
        }
    };
    return voices;
}

const TtsVoice* TtsVoiceRegistry::findVoiceById(const std::string& id) noexcept {
    for (const auto& voice : getAllVoices()) {
        if (voice.id == id) {
            return &voice;
        }
    }
    return nullptr;
}

std::vector<const TtsVoice*> TtsVoiceRegistry::getVoicesByLanguage(const std::string& language) {
    std::vector<const TtsVoice*> result;
    for (const auto& voice : getAllVoices()) {
        if (voice.language == language || voice.langCode == language) {
            result.push_back(&voice);
        }
    }
    return result;
}

std::vector<const TtsVoice*> TtsVoiceRegistry::getVoicesByCategory(const std::string& category) {
    std::vector<const TtsVoice*> result;
    for (const auto& voice : getAllVoices()) {
        if (category == "all") {
            result.push_back(&voice);
            continue;
        }
        for (const auto& cat : voice.categories) {
            if (cat == category) {
                result.push_back(&voice);
                break;
            }
        }
    }
    return result;
}

std::vector<const TtsVoice*> TtsVoiceRegistry::getVoicesByGender(const std::string& gender) {
    std::vector<const TtsVoice*> result;
    for (const auto& voice : getAllVoices()) {
        if (voice.gender == gender) {
            result.push_back(&voice);
        }
    }
    return result;
}

} // namespace catchim::media
