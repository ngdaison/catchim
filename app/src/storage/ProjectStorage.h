#pragma once

#include "editor/project/Project.h"
#include "core/errors/Errors.h"
#include <filesystem>
#include <vector>

namespace catchim::storage {

class ProjectStorage {
public:
    static core::Result<editor::Project> load(const std::filesystem::path& path);

    // Atomic write: .tmp -> flush -> validate -> replace
    static core::Result<void> saveAtomic(const editor::Project& project, const std::filesystem::path& path);

    static bool deleteProject(const std::filesystem::path& path);

    static std::filesystem::path getDefaultProjectsDirectory();
    static std::vector<std::filesystem::path> listProjects();
};

} // namespace catchim::storage
