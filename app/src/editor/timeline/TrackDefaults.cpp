#include "editor/timeline/TrackDefaults.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

static const std::string s_videoTrack = "Video track";
static const std::string s_textTrack = "Text track";
static const std::string s_audioTrack = "Audio track";
static const std::string s_graphicTrack = "Graphic track";
static const std::string s_effectTrack = "Effect track";

const std::string& TrackDefaults::defaultVideoTrackName() noexcept { return s_videoTrack; }
const std::string& TrackDefaults::defaultTextTrackName() noexcept { return s_textTrack; }
const std::string& TrackDefaults::defaultAudioTrackName() noexcept { return s_audioTrack; }
const std::string& TrackDefaults::defaultGraphicTrackName() noexcept { return s_graphicTrack; }
const std::string& TrackDefaults::defaultEffectTrackName() noexcept { return s_effectTrack; }

std::string TrackDefaults::getDefaultTrackName(const std::string& trackType) {
    if (trackType == "video") return s_videoTrack;
    if (trackType == "text") return s_textTrack;
    if (trackType == "audio") return s_audioTrack;
    if (trackType == "graphic") return s_graphicTrack;
    if (trackType == "effect") return s_effectTrack;
    return "Track";
}

double TrackDefaults::linearToDb(double linear) noexcept {
    if (linear <= 0.001) {
        return VOLUME_DB_MIN;
    }
    double db = 20.0 * std::log10(linear);
    return std::clamp(db, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

double TrackDefaults::dbToLinear(double db) noexcept {
    if (db <= VOLUME_DB_MIN) {
        return 0.0;
    }
    return std::pow(10.0, db / 20.0);
}

double TrackDefaults::clampDb(double db) noexcept {
    return std::clamp(db, VOLUME_DB_MIN, VOLUME_DB_MAX);
}

double TrackDefaults::clampLinear(double linear) noexcept {
    return std::clamp(linear, 0.0, 2.0);
}

} // namespace catchim::editor
