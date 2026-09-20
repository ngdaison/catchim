#pragma once

#include "editor/project/Project.h"
#include "editor/project/ProjectSerializer.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

namespace catchim::storage {

struct BundleValidationReport {
    size_t totalMediaReferences{0};
    size_t foundCount{0};
    size_t missingCount{0};
    std::vector<std::string> missingIds;

    [[nodiscard]] bool isComplete() const noexcept {
        return missingCount == 0;
    }
};

class ProjectBundle {
public:
    static BundleValidationReport validateMediaReferences(
        const editor::Project& project,
        const std::filesystem::path& mediaSearchDir
    );

    static bool createBundle(
        const editor::Project& project,
        const std::filesystem::path& targetBundleDir,
        const std::filesystem::path& sourceMediaDir
    );

    static size_t relinkMedia(
        nlohmann::json& projectJson,
        const std::unordered_map<std::string, std::string>& oldToNewPaths
    );
};

} // namespace catchim::storage
