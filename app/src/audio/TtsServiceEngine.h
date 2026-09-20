#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace catchim::audio {

struct TtsSynthesisOptions {
    std::string text;
    std::string voiceId;
    double speed{1.0};   // 0.5 to 2.0 (default 1.0)
    double pitch{0.0};   // -50.0 to 50.0 (default 0.0)
    double volume{1.0};  // 0.0 to 1.0 (default 1.0)

    bool operator==(const TtsSynthesisOptions& other) const = default;
};

class TtsServiceEngine {
public:
    static constexpr double MIN_SPEED = 0.5;
    static constexpr double MAX_SPEED = 2.0;
    static constexpr double DEFAULT_SPEED = 1.0;

    static constexpr double MIN_PITCH = -50.0;
    static constexpr double MAX_PITCH = 50.0;
    static constexpr double DEFAULT_PITCH = 0.0;

    static constexpr double MIN_VOLUME = 0.0;
    static constexpr double MAX_VOLUME = 1.0;
    static constexpr double DEFAULT_VOLUME = 1.0;

    static bool validateOptions(TtsSynthesisOptions& options);

    static std::vector<uint8_t> encodePcmToWav(
        const std::vector<float>& monoSamples,
        uint32_t sampleRate = 44100
    );

    static double calculateAudioDuration(
        size_t sampleCount,
        uint32_t sampleRate = 44100
    ) noexcept;
};

} // namespace catchim::audio
