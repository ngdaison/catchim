#pragma once

#include "core/time/TimelineTime.h"
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::storage {

struct StorageConfig {
    std::string projectsDb{"video-editor-projects"};
    std::string mediaDb{"video-editor-media"};
    std::string savedSoundsDb{"video-editor-saved-sounds"};
    int version{1};

    bool operator==(const StorageConfig& other) const = default;
};

struct NormalizedBookmark {
    core::TimelineTime time{core::TimelineTime::zero()};
    std::optional<std::string> note{std::nullopt};
    std::optional<std::string> color{std::nullopt};
    std::optional<core::TimelineTime> duration{std::nullopt};

    bool operator==(const NormalizedBookmark& other) const = default;
};

class StorageServiceCoordinator {
public:
    static const StorageConfig& defaultConfig() noexcept;

    static std::string getProjectMediaStoreName(
        std::string_view projectId,
        const StorageConfig& config = defaultConfig()
    );

    static std::string getProjectMediaFilesFolder(
        std::string_view projectId
    );

    static std::vector<NormalizedBookmark> normalizeBookmarks(
        const nlohmann::json& raw
    );

    static bool isQuotaExceededError(
        std::string_view errorMessage
    ) noexcept;
};

} // namespace catchim::storage
