#include "TtsServiceEngine.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace catchim::audio {

namespace {

void writeUint32LE(uint8_t* dst, uint32_t val) noexcept {
    dst[0] = static_cast<uint8_t>(val & 0xFF);
    dst[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
    dst[2] = static_cast<uint8_t>((val >> 16) & 0xFF);
    dst[3] = static_cast<uint8_t>((val >> 24) & 0xFF);
}

void writeUint16LE(uint8_t* dst, uint16_t val) noexcept {
    dst[0] = static_cast<uint8_t>(val & 0xFF);
    dst[1] = static_cast<uint8_t>((val >> 8) & 0xFF);
}

void writeInt16LE(uint8_t* dst, int16_t val) noexcept {
    writeUint16LE(dst, static_cast<uint16_t>(val));
}

} // namespace

bool TtsServiceEngine::validateOptions(TtsSynthesisOptions& options) {
    // Trim text
    size_t first = options.text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        options.text.clear();
        return false;
    }
    size_t last = options.text.find_last_not_of(" \t\r\n");
    options.text = options.text.substr(first, last - first + 1);

    options.speed = std::clamp(options.speed, MIN_SPEED, MAX_SPEED);
    options.pitch = std::clamp(options.pitch, MIN_PITCH, MAX_PITCH);
    options.volume = std::clamp(options.volume, MIN_VOLUME, MAX_VOLUME);

    return !options.text.empty();
}

std::vector<uint8_t> TtsServiceEngine::encodePcmToWav(
    const std::vector<float>& monoSamples,
    uint32_t sampleRate
) {
    uint16_t numChannels = 1;
    uint16_t bitsPerSample = 16;
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);
    uint32_t byteRate = sampleRate * blockAlign;
    uint32_t dataLength = static_cast<uint32_t>(monoSamples.size() * blockAlign);
    uint32_t totalLength = 44 + dataLength;

    std::vector<uint8_t> wav(totalLength);
    uint8_t* ptr = wav.data();

    // RIFF chunk
    std::memcpy(ptr + 0, "RIFF", 4);
    writeUint32LE(ptr + 4, 36 + dataLength);
    std::memcpy(ptr + 8, "WAVE", 4);

    // fmt subchunk
    std::memcpy(ptr + 12, "fmt ", 4);
    writeUint32LE(ptr + 16, 16);             // Subchunk1Size for PCM
    writeUint16LE(ptr + 20, 1);              // AudioFormat 1 = PCM
    writeUint16LE(ptr + 22, numChannels);
    writeUint32LE(ptr + 24, sampleRate);
    writeUint32LE(ptr + 28, byteRate);
    writeUint16LE(ptr + 32, blockAlign);
    writeUint16LE(ptr + 34, bitsPerSample);

    // data subchunk
    std::memcpy(ptr + 36, "data", 4);
    writeUint32LE(ptr + 40, dataLength);

    size_t offset = 44;
    for (float sample : monoSamples) {
        float clamped = std::clamp(sample, -1.0f, 1.0f);
        int16_t intSample = (clamped < 0.0f)
            ? static_cast<int16_t>(clamped * 32768.0f)
            : static_cast<int16_t>(clamped * 32767.0f);
        writeInt16LE(ptr + offset, intSample);
        offset += 2;
    }

    return wav;
}

double TtsServiceEngine::calculateAudioDuration(
    size_t sampleCount,
    uint32_t sampleRate
) noexcept {
    if (sampleRate == 0) return 0.0;
    return static_cast<double>(sampleCount) / static_cast<double>(sampleRate);
}

} // namespace catchim::audio
