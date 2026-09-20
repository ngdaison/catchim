#include "ProjectOrganizationEngine.h"
#include <algorithm>
#include <cctype>
#include <charconv>

namespace catchim::core {

namespace {

std::string toLowerString(std::string_view sv) {
    std::string s;
    s.reserve(sv.size());
    for (char c : sv) {
        s.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return s;
}

} // namespace

std::string ProjectOrganizationEngine::sortOptionToString(ProjectSortOption option) noexcept {
    switch (option) {
        case ProjectSortOption::NameAsc: return "name-asc";
        case ProjectSortOption::NameDesc: return "name-desc";
        case ProjectSortOption::UpdatedAtAsc: return "updatedAt-asc";
        case ProjectSortOption::UpdatedAtDesc: return "updatedAt-desc";
        case ProjectSortOption::CreatedAtAsc: return "createdAt-asc";
        case ProjectSortOption::CreatedAtDesc: return "createdAt-desc";
    }
    return "updatedAt-desc";
}

std::optional<ProjectSortOption> ProjectOrganizationEngine::sortOptionFromString(std::string_view str) noexcept {
    if (str == "name-asc") return ProjectSortOption::NameAsc;
    if (str == "name-desc") return ProjectSortOption::NameDesc;
    if (str == "updatedAt-asc") return ProjectSortOption::UpdatedAtAsc;
    if (str == "updatedAt-desc") return ProjectSortOption::UpdatedAtDesc;
    if (str == "createdAt-asc") return ProjectSortOption::CreatedAtAsc;
    if (str == "createdAt-desc") return ProjectSortOption::CreatedAtDesc;
    return std::nullopt;
}

std::vector<ProjectSummary> ProjectOrganizationEngine::filterAndSortProjects(
    const std::vector<ProjectSummary>& projects,
    std::string_view searchQuery,
    ProjectSortOption sortOption
) {
    std::vector<ProjectSummary> result;
    std::string lowerQuery = toLowerString(searchQuery);

    for (const auto& p : projects) {
        if (lowerQuery.empty() || toLowerString(p.name).find(lowerQuery) != std::string::npos) {
            result.push_back(p);
        }
    }

    auto comparator = [sortOption](const ProjectSummary& a, const ProjectSummary& b) {
        switch (sortOption) {
            case ProjectSortOption::NameAsc:
                return a.name < b.name;
            case ProjectSortOption::NameDesc:
                return a.name > b.name;
            case ProjectSortOption::UpdatedAtAsc:
                return a.updatedAtMs < b.updatedAtMs;
            case ProjectSortOption::UpdatedAtDesc:
                return a.updatedAtMs > b.updatedAtMs;
            case ProjectSortOption::CreatedAtAsc:
                return a.createdAtMs < b.createdAtMs;
            case ProjectSortOption::CreatedAtDesc:
                return a.createdAtMs > b.createdAtMs;
        }
        return a.updatedAtMs > b.updatedAtMs;
    };

    std::sort(result.begin(), result.end(), comparator);
    return result;
}

std::pair<std::string, std::optional<int>> ProjectOrganizationEngine::parseDuplicateBaseName(
    std::string_view name
) {
    if (name.size() > 4 && name.front() == '(') {
        size_t closeParen = name.find(')');
        if (closeParen != std::string_view::npos && closeParen + 1 < name.size() && name[closeParen + 1] == ' ') {
            std::string_view numStr = name.substr(1, closeParen - 1);
            int val = 0;
            auto res = std::from_chars(numStr.data(), numStr.data() + numStr.size(), val);
            if (res.ec == std::errc() && res.ptr == numStr.data() + numStr.size() && val > 0) {
                std::string baseName(name.substr(closeParen + 2));
                return {baseName, val};
            }
        }
    }
    return {std::string(name), std::nullopt};
}

std::string ProjectOrganizationEngine::generateDuplicateName(
    std::string_view sourceName,
    const std::vector<std::string>& existingNames
) {
    auto [baseName, srcNum] = parseDuplicateBaseName(sourceName);

    int maxNum = 0;
    for (const auto& existing : existingNames) {
        auto [exBase, exNum] = parseDuplicateBaseName(existing);
        if (exBase == baseName && exNum.has_value()) {
            if (*exNum > maxNum) {
                maxNum = *exNum;
            }
        }
    }

    int nextNum = (maxNum > 0) ? (maxNum + 1) : 1;
    return "(" + std::to_string(nextNum) + ") " + baseName;
}

void ProjectOrganizationEngine::stripAudioBuffers(nlohmann::json& projectJson) noexcept {
    if (!projectJson.is_object()) return;

    if (projectJson.contains("scenes") && projectJson["scenes"].is_array()) {
        for (auto& scene : projectJson["scenes"]) {
            if (!scene.is_object()) continue;

            if (scene.contains("tracks") && scene["tracks"].is_object()) {
                auto& tracks = scene["tracks"];
                if (tracks.contains("audio") && tracks["audio"].is_array()) {
                    for (auto& track : tracks["audio"]) {
                        if (!track.is_object() || !track.contains("elements") || !track["elements"].is_array()) {
                            continue;
                        }
                        for (auto& el : track["elements"]) {
                            if (el.is_object() && el.contains("buffer")) {
                                el.erase("buffer");
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace catchim::core
