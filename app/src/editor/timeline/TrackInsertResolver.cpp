#include "editor/timeline/TrackInsertResolver.h"
#include <algorithm>

namespace catchim::editor {

int TrackInsertResolver::getDefaultInsertIndexForTrack(
    size_t overlayCount,
    size_t audioCount,
    const std::string& trackType
) noexcept {
    if (trackType == "audio") {
        return static_cast<int>(overlayCount + 1 + audioCount);
    }
    if (trackType == "effect") {
        return 0;
    }
    return static_cast<int>(overlayCount);
}

int TrackInsertResolver::getHighestInsertIndexForTrack(
    size_t overlayCount,
    const std::string& trackType
) noexcept {
    if (trackType == "audio") {
        return static_cast<int>(overlayCount + 1);
    }
    return 0;
}

NewTrackPlacementResult TrackInsertResolver::resolvePreferredNewTrackPlacement(
    size_t overlayCount,
    size_t audioCount,
    const std::string& trackType,
    int preferredIndex,
    const std::string& direction
) {
    const int trackCount = static_cast<int>(overlayCount + 1 + audioCount);
    if (trackCount <= 0) {
        return { 0, trackType == "audio" ? "below" : "" };
    }

    const int safePreferredIndex = std::clamp(preferredIndex, 0, trackCount - 1);
    const int mainTrackIndex = static_cast<int>(overlayCount);

    if (trackType == "audio") {
        if (safePreferredIndex <= mainTrackIndex) {
            return { mainTrackIndex + 1, "below" };
        }
        return {
            direction == "above" ? safePreferredIndex : safePreferredIndex + 1,
            direction
        };
    }

    const int insertIndex = (direction == "above") ? safePreferredIndex : (safePreferredIndex + 1);
    if (mainTrackIndex >= 0 && insertIndex > mainTrackIndex) {
        return { mainTrackIndex, "above" };
    }

    return { insertIndex, direction };
}

} // namespace catchim::editor
