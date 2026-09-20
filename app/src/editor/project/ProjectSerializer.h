#pragma once

#include "Project.h"
#include "core/errors/Errors.h"
#include <string>
#include <nlohmann/json.hpp>

namespace catchim::editor {

class ProjectSerializer {
public:
    static nlohmann::json toJson(const Project& project);
    static core::Result<Project> fromJson(const nlohmann::json& j);

    static std::string serialize(const Project& project, int indent = 2);
    static core::Result<Project> deserialize(std::string_view jsonStr);
};

} // namespace catchim::editor
