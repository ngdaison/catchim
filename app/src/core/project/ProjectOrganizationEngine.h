#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace catchim::core {

enum class ProjectSortOption {
    NameAsc,
    NameDesc,
    UpdatedAtAsc,
    UpdatedAtDesc,
    CreatedAtAsc,
    CreatedAtDesc
};

struct ProjectSummary {
    std::string id;
    std::string name;
    int64_t createdAtMs{0};
    int64_t updatedAtMs{0};
    double durationSeconds{0.0};
    std::string thumbnail;

    bool operator==(const ProjectSummary& other) const = default;
};

class ProjectOrganizationEngine {
public:
    static std::string sortOptionToString(ProjectSortOption option) noexcept;
    static std::optional<ProjectSortOption> sortOptionFromString(std::string_view str) noexcept;

    static std::vector<ProjectSummary> filterAndSortProjects(
        const std::vector<ProjectSummary>& projects,
        std::string_view searchQuery,
        ProjectSortOption sortOption
    );

    static std::pair<std::string, std::optional<int>> parseDuplicateBaseName(
        std::string_view name
    );

    static std::string generateDuplicateName(
        std::string_view sourceName,
        const std::vector<std::string>& existingNames
    );

    static void stripAudioBuffers(nlohmann::json& projectJson) noexcept;
};

} // namespace catchim::core
