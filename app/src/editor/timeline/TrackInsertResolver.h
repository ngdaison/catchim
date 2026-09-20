#pragma once

#include <string>
#include <cstddef>

namespace catchim::editor {

struct NewTrackPlacementResult {
    int insertIndex{0};
    std::string insertPosition{""}; // "above", "below", or ""

    bool operator==(const NewTrackPlacementResult& other) const noexcept {
        return insertIndex == other.insertIndex && insertPosition == other.insertPosition;
    }
    bool operator!=(const NewTrackPlacementResult& other) const noexcept {
        return !(*this == other);
    }
};

class TrackInsertResolver {
public:
    static int getDefaultInsertIndexForTrack(
        size_t overlayCount,
        size_t audioCount,
        const std::string& trackType
    ) noexcept;

    static int getHighestInsertIndexForTrack(
        size_t overlayCount,
        const std::string& trackType
    ) noexcept;

    static NewTrackPlacementResult resolvePreferredNewTrackPlacement(
        size_t overlayCount,
        size_t audioCount,
        const std::string& trackType,
        int preferredIndex,
        const std::string& direction
    );
};

} // namespace catchim::editor
