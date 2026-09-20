#include "editor/timeline/TimelineLayoutEngine.h"
#include <algorithm>
#include <unordered_map>

namespace catchim::editor {

int TimelineLayoutEngine::getTrackHeight(TrackType type) noexcept {
    switch (type) {
        case TrackType::Video: return TimelineLayoutMetrics::TRACK_HEIGHT_VIDEO;
        case TrackType::Audio: return TimelineLayoutMetrics::TRACK_HEIGHT_AUDIO;
        case TrackType::Text: return TimelineLayoutMetrics::TRACK_HEIGHT_TEXT;
        case TrackType::Graphic: return TimelineLayoutMetrics::TRACK_HEIGHT_GRAPHIC;
        case TrackType::Effect: return TimelineLayoutMetrics::TRACK_HEIGHT_EFFECT;
    }
    return TimelineLayoutMetrics::TRACK_HEIGHT_VIDEO;
}

int TimelineLayoutEngine::getExpandedTrackHeight(TrackType type, size_t expandedLaneCount) noexcept {
    return getTrackHeight(type) + static_cast<int>(expandedLaneCount) * TimelineLayoutMetrics::KEYFRAME_LANE_HEIGHT;
}

int TimelineLayoutEngine::getCumulativeHeightBefore(
    const std::vector<TrackType>& tracks,
    size_t trackIndex,
    const std::vector<int>& extraHeights
) noexcept {
    int sum = 0;
    size_t limit = std::min(trackIndex, tracks.size());
    for (size_t i = 0; i < limit; ++i) {
        int extra = (i < extraHeights.size()) ? extraHeights[i] : 0;
        sum += getTrackHeight(tracks[i]) + extra + TimelineLayoutMetrics::TIMELINE_TRACK_GAP;
    }
    return sum;
}

std::vector<int> TimelineLayoutEngine::getTrackLayoutOffsets(
    const std::vector<TrackType>& tracks,
    const std::vector<int>& extraHeights
) noexcept {
    std::vector<int> offsets;
    offsets.reserve(tracks.size());
    int currentOffset = 0;

    for (size_t i = 0; i < tracks.size(); ++i) {
        offsets.push_back(currentOffset);
        int extra = (i < extraHeights.size()) ? extraHeights[i] : 0;
        currentOffset += getTrackHeight(tracks[i]) + extra + TimelineLayoutMetrics::TIMELINE_TRACK_GAP;
    }
    return offsets;
}

int TimelineLayoutEngine::getTotalTracksHeight(
    const std::vector<TrackType>& tracks,
    const std::vector<int>& extraHeights
) noexcept {
    if (tracks.empty()) return 0;
    int tracksHeight = 0;
    for (size_t i = 0; i < tracks.size(); ++i) {
        int extra = (i < extraHeights.size()) ? extraHeights[i] : 0;
        tracksHeight += getTrackHeight(tracks[i]) + extra;
    }
    int gapsHeight = static_cast<int>(tracks.size() - 1) * TimelineLayoutMetrics::TIMELINE_TRACK_GAP;
    return tracksHeight + gapsHeight;
}

std::string TimelineLayoutEngine::getPropertyLabel(const std::string& path) noexcept {
    static const std::unordered_map<std::string, std::string> s_labels = {
        {"transform.positionX", "Position X"},
        {"transform.positionY", "Position Y"},
        {"transform.scaleX", "Scale X"},
        {"transform.scaleY", "Scale Y"},
        {"transform.rotate", "Rotation"},
        {"opacity", "Opacity"},
        {"volume", "Volume"},
        {"color", "Color"},
        {"background.color", "BG Color"},
        {"background.paddingX", "BG Pad X"},
        {"background.paddingY", "BG Pad Y"},
        {"background.offsetX", "BG Offset X"},
        {"background.offsetY", "BG Offset Y"},
        {"background.cornerRadius", "Corner Radius"}
    };

    auto it = s_labels.find(path);
    if (it != s_labels.end()) {
        return it->second;
    }
    if (path.rfind("params.", 0) == 0) {
        return path.substr(7);
    }
    if (path.rfind("effects.", 0) == 0) {
        size_t dot = path.rfind('.');
        if (dot != std::string::npos && dot + 1 < path.size()) {
            return path.substr(dot + 1);
        }
    }
    return path;
}

std::vector<ExpandedRow> TimelineLayoutEngine::getExpandedRows(const Clip& clip) {
    std::vector<ExpandedRow> rows;
    if (!clip.params().contains("animations") || !clip.params()["animations"].is_object()) {
        return rows;
    }

    const auto& animObj = clip.params()["animations"];

    // Preferred group ordering
    static const std::vector<std::string> standardOrder = {
        "transform.positionX",
        "transform.positionY",
        "transform.scaleX",
        "transform.scaleY",
        "transform.rotate",
        "opacity",
        "volume",
        "color",
        "background.color",
        "background.paddingX",
        "background.paddingY",
        "background.offsetX",
        "background.offsetY",
        "background.cornerRadius"
    };

    for (const auto& prop : standardOrder) {
        if (animObj.contains(prop)) {
            rows.push_back({prop, getPropertyLabel(prop)});
        }
    }

    for (auto it = animObj.begin(); it != animObj.end(); ++it) {
        const std::string& key = it.key();
        if (std::find(standardOrder.begin(), standardOrder.end(), key) == standardOrder.end()) {
            rows.push_back({key, getPropertyLabel(key)});
        }
    }

    return rows;
}

int TimelineLayoutEngine::getExpansionHeight(const std::vector<ExpandedRow>& rows) noexcept {
    return static_cast<int>(rows.size()) * TimelineLayoutMetrics::KEYFRAME_LANE_HEIGHT;
}

int TimelineLayoutEngine::computeTrackExpansionHeight(
    const Track& track,
    const std::unordered_set<core::ClipId>& expandedClipIds
) {
    int maxHeight = 0;
    for (const auto& clip : track.clips()) {
        if (expandedClipIds.find(clip.id()) == expandedClipIds.end()) continue;
        auto rows = getExpandedRows(clip);
        int h = getExpansionHeight(rows);
        if (h > maxHeight) {
            maxHeight = h;
        }
    }
    return maxHeight;
}

std::vector<ExpandedRow> TimelineLayoutEngine::getTrackExpandedRows(
    const Track& track,
    const std::unordered_set<core::ClipId>& expandedClipIds
) {
    int maxHeight = 0;
    std::vector<ExpandedRow> maxRows;

    for (const auto& clip : track.clips()) {
        if (expandedClipIds.find(clip.id()) == expandedClipIds.end()) continue;
        auto rows = getExpandedRows(clip);
        int h = getExpansionHeight(rows);
        if (h > maxHeight) {
            maxHeight = h;
            maxRows = std::move(rows);
        }
    }
    return maxRows;
}

} // namespace catchim::editor
