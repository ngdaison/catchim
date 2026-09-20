#pragma once

namespace catchim::audio {

class AudioDisplayMetrics {
public:
    static constexpr double VOLUME_DB_MIN = -60.0;
    static constexpr double VOLUME_DB_MAX = 20.0;
    static constexpr double MIN_DISPLAY_DB = -40.0;
    static constexpr double SLIDER_CURVE_EXPONENT = 2.0;
    static constexpr double WAVEFORM_BAR_EXPONENT = 1.5;

    static double clampDb(double db) noexcept;
    static double getNormalizedGainFromDb(double db) noexcept;

    // Maps volume setting in dB to line position percentage (0.0% to 100.0%)
    static double getLinePosFromDb(double db) noexcept;

    // Inverse: converts line position percentage back to volume in dB
    static double getDbFromLinePos(double percent) noexcept;

    // Maps raw output sample amplitude to visible waveform bar height fraction (0.0 to 1.0)
    static double getBarFractionFromOutputAmplitude(double outputAmplitude) noexcept;
};

} // namespace catchim::audio
