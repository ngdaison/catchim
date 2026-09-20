#pragma once

#include <string>
#include <unordered_map>

namespace catchim::editor {

class TrackDefaults {
public:
    static constexpr double VOLUME_DB_MIN = -60.0;
    static constexpr double VOLUME_DB_MAX = 20.0;

    static const std::string& defaultVideoTrackName() noexcept;
    static const std::string& defaultTextTrackName() noexcept;
    static const std::string& defaultAudioTrackName() noexcept;
    static const std::string& defaultGraphicTrackName() noexcept;
    static const std::string& defaultEffectTrackName() noexcept;

    static std::string getDefaultTrackName(const std::string& trackType);

    // Audio dB / Linear conversion utilities
    static double linearToDb(double linear) noexcept;
    static double dbToLinear(double db) noexcept;
    static double clampDb(double db) noexcept;
    static double clampLinear(double linear) noexcept;
};

} // namespace catchim::editor
