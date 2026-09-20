#pragma once

#include "editor/timeline/Clip.h"
#include <vector>
#include <cmath>
#include <algorithm>

namespace catchim::audio {

constexpr double kVolumeDbMin = -60.0;
constexpr double kVolumeDbMax = 12.0;
constexpr double kDefaultGainStepSeconds = 1.0 / 60.0;

struct AudioAutomationPoint {
    double localTime{0.0};
    double gain{1.0};

    bool operator==(const AudioAutomationPoint& other) const {
        return std::abs(localTime - other.localTime) < 1e-6 &&
               std::abs(gain - other.gain) < 1e-6;
    }
};

class AudioStateEngine {
public:
    static double clampDb(double value) noexcept;
    static double dBToLinear(double db) noexcept;
    static double linearToDb(double linear) noexcept;

    static double getElementVolume(const editor::Clip& clip) noexcept;
    static bool isElementMuted(const editor::Clip& clip) noexcept;
    static bool hasAnimatedVolume(const editor::Clip& clip) noexcept;

    static double resolveEffectiveAudioGain(
        const editor::Clip& clip,
        bool trackMuted = false,
        double localTimeSeconds = 0.0
    ) noexcept;

    static std::vector<double> buildWaveformGainSamples(
        const editor::Clip& clip,
        int count
    );

    static std::vector<AudioAutomationPoint> buildAudioGainAutomation(
        const editor::Clip& clip,
        bool trackMuted = false,
        double fromLocalTime = 0.0,
        double toLocalTime = 0.0,
        double stepSeconds = kDefaultGainStepSeconds
    );
};

} // namespace catchim::audio
