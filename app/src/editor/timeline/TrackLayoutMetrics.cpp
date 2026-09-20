#include "TrackLayoutMetrics.h"
#include <algorithm>

namespace catchim::editor {

double TrackLayoutMetrics::getTrackHeight(TrackType type) noexcept {
    switch (type) {
        case TrackType::Video: return TRACK_HEIGHT_VIDEO;
        case TrackType::Audio: return TRACK_HEIGHT_AUDIO;
        case TrackType::Text: return TRACK_HEIGHT_TEXT;
        case TrackType::Graphic: return TRACK_HEIGHT_GRAPHIC;
        case TrackType::Effect: return TRACK_HEIGHT_EFFECT;
    }
    return TRACK_HEIGHT_VIDEO;
}

double TrackLayoutMetrics::getExpandedTrackHeight(TrackType type, size_t expandedLaneCount) noexcept {
    return getTrackHeight(type) + static_cast<double>(expandedLaneCount) * KEYFRAME_LANE_HEIGHT;
}

double TrackLayoutMetrics::getCumulativeHeightBefore(
    const std::vector<const Track*>& tracks,
    size_t trackIndex,
    const std::function<double(size_t)>& getExtraHeight
) noexcept {
    double sum = 0.0;
    const size_t limit = std::min(trackIndex, tracks.size());
    for (size_t i = 0; i < limit; ++i) {
        if (tracks[i]) {
            sum += getTrackHeight(tracks[i]->type());
            if (getExtraHeight) {
                sum += getExtraHeight(i);
            }
            sum += TIMELINE_TRACK_GAP;
        }
    }
    return sum;
}

std::vector<double> TrackLayoutMetrics::getTrackLayoutOffsets(
    const std::vector<const Track*>& tracks,
    const std::function<double(size_t)>& getExtraHeight
) noexcept {
    std::vector<double> offsets;
    offsets.reserve(tracks.size());
    double currentOffset = 0.0;

    for (size_t i = 0; i < tracks.size(); ++i) {
        offsets.push_back(currentOffset);
        if (tracks[i]) {
            currentOffset += getTrackHeight(tracks[i]->type()) +
                             (getExtraHeight ? getExtraHeight(i) : 0.0) +
                             TIMELINE_TRACK_GAP;
        }
    }

    return offsets;
}

double TrackLayoutMetrics::getTotalTracksHeight(
    const std::vector<const Track*>& tracks,
    const std::function<double(size_t)>& getExtraHeight
) noexcept {
    if (tracks.empty()) return 0.0;

    double tracksHeight = 0.0;
    for (size_t i = 0; i < tracks.size(); ++i) {
        if (tracks[i]) {
            tracksHeight += getTrackHeight(tracks[i]->type()) + (getExtraHeight ? getExtraHeight(i) : 0.0);
        }
    }

    const double gapsHeight = static_cast<double>(tracks.size() - 1) * TIMELINE_TRACK_GAP;
    return tracksHeight + gapsHeight;
}

} // namespace catchim::editor
