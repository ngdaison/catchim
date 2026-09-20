#include "audio/TtsEngine.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace catchim::audio {

TtsEngine& TtsEngine::instance() {
    static TtsEngine s_instance;
    return s_instance;
}

TtsEngine::TtsEngine() {
    initVoices();
}

void TtsEngine::initVoices() {
    voices_ = {
        {
            "vi-female-sweet",
            "Hoài My (Nữ miền Bắc)",
            "vi-VN",
            "female",
            "Giọng nữ miền Bắc ngọt ngào, trong trẻo, truyền cảm chuẩn TikTok",
            "Xin chào! Mình là Hoài My, giọng nữ Bắc ngọt ngào và tự nhiên.",
            1.0,
            1.0
        },
        {
            "vi-male-warm",
            "Nam Minh (Nam miền Bắc)",
            "vi-VN",
            "male",
            "Giọng nam miền Bắc trầm ấm, đĩnh đạc, phát thanh viên chuyên nghiệp",
            "Chào bạn, đây là giọng nam trầm ấm Nam Minh, rất vui được đồng hành cùng bạn.",
            1.0,
            1.0
        },
        {
            "vi-female-google",
            "Chị Google (Việt Nam)",
            "vi-VN",
            "female",
            "Giọng đọc Google nguyên bản quen thuộc, kinh điển trên mạng xã hội",
            "Xin chào các bạn! Tôi là chị Google, giọng đọc huyền thoại của các video viral.",
            1.0,
            1.0
        },
        {
            "en-intl-andrew",
            "Andrew (Đa ngôn ngữ Nam)",
            "en-US",
            "male",
            "Giọng nam AI đa ngôn ngữ phong cách trẻ trung, hiện đại",
            "Xin chào, tôi là Andrew, giọng đọc AI đa ngôn ngữ hiện đại.",
            1.0,
            1.0
        },
        {
            "en-intl-ava",
            "Ava (Đa ngôn ngữ Nữ)",
            "en-US",
            "female",
            "Giọng nữ AI đa ngôn ngữ tươi sáng, năng động",
            "Xin chào, mình là Ava, giọng đọc phong cách sống tươi tắn.",
            1.0,
            1.0
        },
        {
            "en-intl-brian",
            "Brian (Đa ngôn ngữ Trầm)",
            "en-US",
            "male",
            "Giọng nam AI đa ngôn ngữ trầm ấm, cuốn hút",
            "Xin chào các bạn, tôi là Brian, giọng đọc trầm ấm phù hợp cho câu chuyện sâu lắng.",
            1.0,
            1.0
        }
    };
}

const TtsVoice* TtsEngine::findVoice(const std::string& id) const noexcept {
    for (const auto& v : voices_) {
        if (v.id == id) return &v;
    }
    return nullptr;
}

std::vector<TtsVoice> TtsEngine::findVoicesByLanguage(const std::string& langPrefix) const {
    std::vector<TtsVoice> result;
    for (const auto& v : voices_) {
        if (v.language.rfind(langPrefix, 0) == 0) {
            result.push_back(v);
        }
    }
    return result;
}

double TtsEngine::estimateSpeechDuration(
    const std::string& text,
    double rateMultiplier,
    const std::string& /*lang*/
) {
    std::istringstream iss(text);
    std::string word;
    size_t wordCount = 0;
    while (iss >> word) {
        ++wordCount;
    }

    if (wordCount == 0) return 0.0;

    double rate = std::clamp(rateMultiplier, 0.25, 4.0);
    double wordsPerSec = 3.5 * rate;

    return std::max(0.6, static_cast<double>(wordCount) / wordsPerSec);
}

std::vector<uint8_t> TtsEngine::encodePcm16Wav(const AudioBuffer& buffer) {
    uint16_t numChannels = static_cast<uint16_t>(std::max(1, buffer.channels()));
    uint32_t sampleRate = static_cast<uint32_t>(std::max(8000, buffer.sampleRate()));
    size_t frameCount = buffer.frameCount();
    uint32_t dataBytes = static_cast<uint32_t>(frameCount * numChannels * sizeof(int16_t));
    uint32_t totalFileSize = 44 + dataBytes;

    std::vector<uint8_t> wav(totalFileSize, 0);

    auto write32 = [&](size_t offset, uint32_t val) {
        wav[offset + 0] = static_cast<uint8_t>(val & 0xFF);
        wav[offset + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
        wav[offset + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
        wav[offset + 3] = static_cast<uint8_t>((val >> 24) & 0xFF);
    };

    auto write16 = [&](size_t offset, uint16_t val) {
        wav[offset + 0] = static_cast<uint8_t>(val & 0xFF);
        wav[offset + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
    };

    // 1. RIFF Header
    std::memcpy(&wav[0], "RIFF", 4);
    write32(4, 36 + dataBytes);
    std::memcpy(&wav[8], "WAVE", 4);

    // 2. fmt chunk
    std::memcpy(&wav[12], "fmt ", 4);
    write32(16, 16); // subchunk1 size
    write16(20, 1);  // PCM format
    write16(22, numChannels);
    write32(24, sampleRate);
    uint32_t byteRate = sampleRate * numChannels * 2;
    write32(28, byteRate);
    uint16_t blockAlign = numChannels * 2;
    write16(32, blockAlign);
    write16(34, 16); // 16 bits per sample

    // 3. data chunk
    std::memcpy(&wav[36], "data", 4);
    write32(40, dataBytes);

    // 4. Sample data
    const auto& samples = buffer.samples();
    size_t outOffset = 44;
    size_t totalSamples = frameCount * numChannels;

    for (size_t i = 0; i < totalSamples && i < samples.size(); ++i) {
        float s = std::clamp(samples[i], -1.0f, 1.0f);
        int16_t pcm = (s < 0.0f) ? static_cast<int16_t>(s * 32768.0f)
                                  : static_cast<int16_t>(s * 32767.0f);
        write16(outOffset, static_cast<uint16_t>(pcm));
        outOffset += 2;
    }

    return wav;
}

} // namespace catchim::audio
