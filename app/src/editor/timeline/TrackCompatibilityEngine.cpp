#include "editor/timeline/TrackCompatibilityEngine.h"

namespace catchim::editor {

std::string TrackCompatibilityEngine::getTrackTypeForElementType(const std::string& elementType) {
    if (elementType == "audio") return "audio";
    if (elementType == "text") return "text";
    if (elementType == "sticker" || elementType == "graphic") return "graphic";
    if (elementType == "effect") return "effect";
    if (elementType == "video" || elementType == "image") return "video";
    return "";
}

bool TrackCompatibilityEngine::canElementGoOnTrack(
    const std::string& elementType,
    const std::string& trackType
) noexcept {
    std::string expectedTrack = getTrackTypeForElementType(elementType);
    if (expectedTrack.empty()) {
        return false;
    }
    return expectedTrack == trackType;
}

CompatibilityResult TrackCompatibilityEngine::validateElementTrackCompatibility(
    const std::string& elementType,
    const std::string& trackType
) {
    if (canElementGoOnTrack(elementType, trackType)) {
        return { true, "" };
    }
    return { false, elementType + " elements cannot be placed on " + trackType + " tracks" };
}

} // namespace catchim::editor
