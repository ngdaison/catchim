#pragma once

#include <string>
#include <unordered_map>

namespace catchim::editor {

struct CompatibilityResult {
    bool isValid{true};
    std::string errorMessage{""};

    bool operator==(const CompatibilityResult& other) const noexcept {
        return isValid == other.isValid && errorMessage == other.errorMessage;
    }
};

class TrackCompatibilityEngine {
public:
    static std::string getTrackTypeForElementType(const std::string& elementType);
    static bool canElementGoOnTrack(const std::string& elementType, const std::string& trackType) noexcept;
    static CompatibilityResult validateElementTrackCompatibility(
        const std::string& elementType,
        const std::string& trackType
    );
};

} // namespace catchim::editor
