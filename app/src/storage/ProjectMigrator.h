#pragma once

#include <nlohmann/json.hpp>

namespace catchim::storage {

class ProjectMigrator {
public:
    static constexpr int TARGET_VERSION = 31;

    // Checks project json version and applies incremental transformations up to TARGET_VERSION
    static nlohmann::json migrate(const nlohmann::json& projectJson);

    static bool needsMigration(const nlohmann::json& projectJson);
};

} // namespace catchim::storage
