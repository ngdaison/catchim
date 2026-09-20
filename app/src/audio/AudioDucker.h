#pragma once

#include "audio/AudioBuffer.h"
#include <vector>

namespace catchim::audio {

struct DuckingConfig {
    double thresholdDb{-24.0};
    double duckingDb{-12.0};
    double attackTimeSec{0.05};
    double holdTimeSec{0.2};
    double releaseTimeSec{0.3};
};

class AudioDucker {
public:
    static void applyDucking(
        AudioBuffer& musicBuffer,
        const AudioBuffer& voiceBuffer,
        const DuckingConfig& config = {}
    ) noexcept;
};

} // namespace catchim::audio
