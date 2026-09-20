#pragma once

#include "editor/project/Project.h"
#include "editor/scene/ScenesManager.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <string>
#include <optional>

namespace catchim::editor {

class SceneHierarchyUtils {
public:
    static const Scene* getMainScene(const std::vector<Scene>& scenes) noexcept;
    static Scene* getMainScene(std::vector<Scene>& scenes) noexcept;

    static void ensureMainScene(std::vector<Scene>& scenes);

    static Scene buildDefaultScene(
        const std::string& name = "Main scene",
        bool isMain = true
    );

    static SceneDeletionResult canDeleteScene(const Scene& scene) noexcept;

    static const Scene* getFallbackSceneAfterDelete(
        const std::vector<Scene>& scenes,
        const core::SceneId& deletedSceneId,
        const std::optional<core::SceneId>& currentSceneId
    ) noexcept;

    static const Scene* findCurrentScene(
        const std::vector<Scene>& scenes,
        const std::optional<core::SceneId>& currentSceneId
    ) noexcept;

    static core::TimelineTime calculateTotalDuration(const Timeline& timeline) noexcept;

    static core::TimelineTime getProjectDurationFromScenes(
        const std::vector<Scene>& scenes
    ) noexcept;
};

} // namespace catchim::editor
