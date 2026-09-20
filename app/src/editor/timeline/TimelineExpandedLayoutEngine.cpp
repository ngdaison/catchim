#include "TimelineExpandedLayoutEngine.h"
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

namespace catchim::editor {

namespace {

bool matchesGroup(int groupIdx, const std::string& path) {
    switch (groupIdx) {
        case 0: // transform.* or opacity
            return path.rfind("transform.", 0) == 0 || path == "opacity";
        case 1: // volume or color
            return path == "volume" || path == "color";
        case 2: // background.*
            return path.rfind("background.", 0) == 0;
        case 3: // params.*
            return path.rfind("params.", 0) == 0;
        case 4: // effects.*
            return path.rfind("effects.", 0) == 0;
        default:
            return false;
    }
}

} // namespace

std::string TimelineExpandedLayoutEngine::getPropertyLabel(const std::string& path) {
    static const std::unordered_map<std::string, std::string> kLabels = {
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

    auto it = kLabels.find(path);
    if (it != kLabels.end()) {
        return it->second;
    }

    if (path.rfind("params.", 0) == 0) {
        return path.substr(7);
    }

    if (path.rfind("effects.", 0) == 0) {
        auto lastDot = path.rfind('.');
        if (lastDot != std::string::npos && lastDot + 1 < path.size()) {
            return path.substr(lastDot + 1);
        }
    }

    return path;
}

std::vector<ExpandedRow> TimelineExpandedLayoutEngine::getExpandedRowsFromPaths(
    const std::vector<std::string>& propertyPaths
) {
    if (propertyPaths.empty()) {
        return {};
    }

    std::vector<std::string> uniquePaths;
    std::unordered_set<std::string> seen;
    for (const auto& p : propertyPaths) {
        if (seen.insert(p).second) {
            uniquePaths.push_back(p);
        }
    }

    std::vector<ExpandedRow> rows;
    for (int groupIdx = 0; groupIdx < 5; ++groupIdx) {
        for (const auto& path : uniquePaths) {
            if (matchesGroup(groupIdx, path)) {
                rows.push_back(ExpandedRow{
                    .propertyPath = path,
                    .label = getPropertyLabel(path)
                });
            }
        }
    }

    // Any remaining paths not matched by the 5 groups
    for (const auto& path : uniquePaths) {
        bool matched = false;
        for (int groupIdx = 0; groupIdx < 5; ++groupIdx) {
            if (matchesGroup(groupIdx, path)) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            rows.push_back(ExpandedRow{
                .propertyPath = path,
                .label = getPropertyLabel(path)
            });
        }
    }

    return rows;
}

std::vector<ExpandedRow> TimelineExpandedLayoutEngine::getExpandedRowsForClip(
    const Clip& clip
) {
    std::vector<std::string> paths;
    for (const auto& [propName, channel] : clip.animationChannels()) {
        if (!channel.keyframes().empty()) {
            paths.push_back(propName);
        }
    }
    return getExpandedRowsFromPaths(paths);
}

double TimelineExpandedLayoutEngine::getExpansionHeight(
    const std::vector<ExpandedRow>& rows
) noexcept {
    return static_cast<double>(rows.size()) * KEYFRAME_LANE_HEIGHT_PX;
}

double TimelineExpandedLayoutEngine::computeTrackExpansionHeight(
    const Track& track,
    const std::unordered_set<std::string>& expandedElementIds
) {
    double maxHeight = 0.0;
    for (const auto& clip : track.clips()) {
        if (!expandedElementIds.contains(clip.id().str())) {
            continue;
        }
        auto rows = getExpandedRowsForClip(clip);
        double h = getExpansionHeight(rows);
        if (h > maxHeight) {
            maxHeight = h;
        }
    }
    return maxHeight;
}

std::vector<ExpandedRow> TimelineExpandedLayoutEngine::getTrackExpandedRows(
    const Track& track,
    const std::unordered_set<std::string>& expandedElementIds
) {
    double maxHeight = 0.0;
    std::vector<ExpandedRow> maxRows;

    for (const auto& clip : track.clips()) {
        if (!expandedElementIds.contains(clip.id().str())) {
            continue;
        }
        auto rows = getExpandedRowsForClip(clip);
        double h = getExpansionHeight(rows);
        if (h > maxHeight) {
            maxHeight = h;
            maxRows = std::move(rows);
        }
    }

    return maxRows;
}

} // namespace catchim::editor
