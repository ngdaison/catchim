#include "audio/AudioDisplayMetrics.h"
#include <cmath>
#include <algorithm>

namespace catchim::audio {

namespace {
const double MIN_LINEAR_GAIN = std::pow(10.0, AudioDisplayMetrics::VOLUME_DB_MIN / 20.0);
const double MAX_LINEAR_GAIN = std::pow(10.0, AudioDisplayMetrics::VOLUME_DB_MAX / 20.0);
const double LINEAR_GAIN_RANGE = MAX_LINEAR_GAIN - MIN_LINEAR_GAIN;
}

double AudioDisplayMetrics::clampDb(double db) noexcept {
    return std::clamp(db, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

double AudioDisplayMetrics::getNormalizedGainFromDb(double db) noexcept {
    double clamped = clampDb(db);
    double linear = std::pow(10.0, clamped / 20.0);
    return (linear - MIN_LINEAR_GAIN) / LINEAR_GAIN_RANGE;
}

double AudioDisplayMetrics::getLinePosFromDb(double db) noexcept {
    double norm = std::clamp(getNormalizedGainFromDb(db), 0.0, 1.0);
    double progress = std::pow(norm, 1.0 / SLIDER_CURVE_EXPONENT);
    return (1.0 - progress) * 100.0;
}

double AudioDisplayMetrics::getDbFromLinePos(double percent) noexcept {
    double clamped = std::clamp(percent, 0.0, 100.0);
    double progress = 1.0 - (clamped / 100.0);
    double norm = std::pow(progress, SLIDER_CURVE_EXPONENT);
    double linear = MIN_LINEAR_GAIN + norm * LINEAR_GAIN_RANGE;
    if (linear <= 1e-12) return VOLUME_DB_MIN;
    return clampDb(20.0 * std::log10(linear));
}

double AudioDisplayMetrics::getBarFractionFromOutputAmplitude(double outputAmplitude) noexcept {
    if (outputAmplitude <= 0.0) return 0.0;
    double db = 20.0 * std::log10(outputAmplitude);
    if (db <= MIN_DISPLAY_DB) return 0.0;
    double ratio = (db - MIN_DISPLAY_DB) / (-MIN_DISPLAY_DB);
    return std::min(1.0, std::pow(std::clamp(ratio, 0.0, 1.0), WAVEFORM_BAR_EXPONENT));
}

} // namespace catchim::audio
