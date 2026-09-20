#include "AudioVolumeLineEngine.h"
#include "AudioDisplayMetrics.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace catchim::audio {

double AudioVolumeLineEngine::clampVolumeDb(double value) noexcept {
    double snapped = std::round(value / VOLUME_STEP) * VOLUME_STEP;
    return std::clamp(snapped, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

double AudioVolumeLineEngine::getLinePositionPercent(double volumeDb) noexcept {
    return AudioDisplayMetrics::getLinePosFromDb(volumeDb);
}

double AudioVolumeLineEngine::getDbFromLinePosition(double percent) noexcept {
    double db = AudioDisplayMetrics::getDbFromLinePos(percent);
    return clampVolumeDb(db);
}

double AudioVolumeLineEngine::getVolumeFromPointer(
    double clientY,
    double rectTop,
    double rectHeight
) noexcept {
    if (rectHeight <= 0.0) {
        return 0.0;
    }
    double clampedOffset = std::clamp(clientY - rectTop, 0.0, rectHeight);
    double progressPercent = (clampedOffset / rectHeight) * 100.0;
    return getDbFromLinePosition(progressPercent);
}

std::string AudioVolumeLineEngine::formatVolumeLabel(double volumeDb) {
    double rounded = clampVolumeDb(volumeDb);
    std::ostringstream ss;
    if (rounded > 0.001) {
        ss << "+";
    } else if (std::abs(rounded) < 0.001) {
        rounded = 0.0;
    }
    ss << std::fixed << std::setprecision(1) << rounded << " dB";
    return ss.str();
}

} // namespace catchim::audio
