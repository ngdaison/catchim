#include "storage/ProjectMigrator.h"
#include "core/logging/Logger.h"

namespace catchim::storage {

bool ProjectMigrator::needsMigration(const nlohmann::json& projectJson) {
    if (!projectJson.is_object()) return false;
    int version = projectJson.value("version", 0);
    return version < TARGET_VERSION;
}

nlohmann::json ProjectMigrator::migrate(const nlohmann::json& projectJson) {
    if (!projectJson.is_object()) {
        return projectJson;
    }

    nlohmann::json root = projectJson;
    int currentVer = root.value("version", 0);

    if (currentVer >= TARGET_VERSION) {
        return root;
    }

    LOG_INFO("Migrating project from schema version {} to {}", currentVer, TARGET_VERSION);

    // 1. Ensure metadata exists
    if (!root.contains("metadata") || !root["metadata"].is_object()) {
        root["metadata"] = nlohmann::json::object();
    }
    if (!root["metadata"].contains("id")) root["metadata"]["id"] = "migrated-project";
    if (!root["metadata"].contains("name")) root["metadata"]["name"] = "Migrated project";
    if (!root["metadata"].contains("thumbnail")) root["metadata"]["thumbnail"] = "";
    if (!root["metadata"].contains("duration")) root["metadata"]["duration"] = 0;

    // 2. Ensure settings exist
    if (!root.contains("settings") || !root["settings"].is_object()) {
        root["settings"] = nlohmann::json::object();
    }
    if (!root["settings"].contains("fps")) {
        root["settings"]["fps"] = {{"numerator", 30}, {"denominator", 1}};
    }
    if (!root["settings"].contains("canvasSize")) {
        root["settings"]["canvasSize"] = {{"width", 1920}, {"height", 1080}};
    }
    if (!root["settings"].contains("canvasSizeMode")) {
        root["settings"]["canvasSizeMode"] = "preset";
    }
    if (!root["settings"].contains("background")) {
        root["settings"]["background"] = {
            {"type", "color"},
            {"color", "#000000"},
            {"blurIntensity", 10.0}
        };
    }

    // 3. Ensure scenes exist
    if (!root.contains("scenes") || !root["scenes"].is_array() || root["scenes"].empty()) {
        nlohmann::json defaultScene = {
            {"id", "scene-1"},
            {"name", "Main scene"},
            {"isMain", true},
            {"bookmarks", nlohmann::json::array()},
            {"tracks", {
                {"overlay", nlohmann::json::array()},
                {"main", {
                    {"id", "main-video-track"},
                    {"name", "Video"},
                    {"type", "video"},
                    {"muted", false},
                    {"hidden", false},
                    {"clips", nlohmann::json::array()}
                }},
                {"audio", nlohmann::json::array()}
            }}
        };

        // If legacy root has tracks array, migrate them
        if (root.contains("tracks") && root["tracks"].is_array()) {
            for (const auto& t : root["tracks"]) {
                std::string type = t.value("type", "video");
                if (type == "audio") {
                    defaultScene["tracks"]["audio"].push_back(t);
                } else {
                    defaultScene["tracks"]["overlay"].push_back(t);
                }
            }
        }

        root["scenes"] = nlohmann::json::array({defaultScene});
        root["currentSceneId"] = "scene-1";
    }

    // 4. Ensure timelineViewState exists
    if (!root.contains("timelineViewState") || !root["timelineViewState"].is_object()) {
        root["timelineViewState"] = {
            {"zoomLevel", 1.0},
            {"scrollLeft", 0.0},
            {"playheadTime", 0}
        };
    }

    root["version"] = TARGET_VERSION;
    LOG_INFO("Project migration to version {} completed successfully", TARGET_VERSION);
    return root;
}

} // namespace catchim::storage
