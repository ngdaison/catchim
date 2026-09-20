#pragma once

#include <vector>
#include <string>

namespace catchim::audio {

struct WaveformBar {
    double x{0.0};
    double top{0.0};
    double height{0.0};
    bool isBurnt{false};

    bool operator==(const WaveformBar& other) const = default;
};

class AudioWaveformBarEngine {
public:
    static constexpr double BAR_WIDTH = 1.0;
    static constexpr double BAR_GAP = 1.0;
    static constexpr double BAR_STEP = 2.0;
    static constexpr size_t WAVEFORM_GAIN_SAMPLE_COUNT = 200;
    static constexpr const char* DEFAULT_WAVEFORM_COLOR = "rgba(255, 255, 255, 0.7)";
    static constexpr const char* WAVEFORM_BURN_COLOR = "rgba(255, 110, 20, 0.9)";

    static double sampleGainAtClipTime(
        const std::vector<double>& samples,
        double clipTimeSec,
        double clipDurationSec
    ) noexcept;

    static std::vector<WaveformBar> calculateWaveformBars(
        const std::vector<float>& amplitudes,
        double width,
        double height,
        const std::vector<double>& gainSamples,
        double clipDurationSec
    );
};

} // namespace catchim::audio
