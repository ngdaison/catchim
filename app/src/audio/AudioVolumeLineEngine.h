#pragma once

#include <string>

namespace catchim::audio {

class AudioVolumeLineEngine {
public:
    static constexpr double HIT_AREA_HEIGHT_PX = 14.0;
    static constexpr double TOOLTIP_OFFSET_PX = 10.0;
    static constexpr double VOLUME_STEP = 0.1;
    static constexpr double VOLUME_DB_MIN = -60.0;
    static constexpr double VOLUME_DB_MAX = 12.0;

    static double clampVolumeDb(double value) noexcept;
    static double getLinePositionPercent(double volumeDb) noexcept;
    static double getDbFromLinePosition(double percent) noexcept;

    static double getVolumeFromPointer(
        double clientY,
        double rectTop,
        double rectHeight
    ) noexcept;

    static std::string formatVolumeLabel(double volumeDb);
};

} // namespace catchim::audio
