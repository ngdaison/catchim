#pragma once

#include "editor/project/Project.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <optional>

namespace catchim::editor {

class TimelineSceneUtils {
public:
    static const Scene* getMainScene(const std::vector<Scene>& scenes) noexcept;
    static Scene* getMainScene(std::vector<Scene>& scenes) noexcept;

    static void ensureMainScene(std::vector<Scene>& scenes);

    static bool canDeleteScene(const Scene& scene) noexcept;

    static const Scene* getFallbackSceneAfterDelete(
        const std::vector<Scene>& scenes,
        const core::SceneId& deletedId,
        const core::SceneId& currentId
    ) noexcept;

    static const Scene* findCurrentScene(
        const std::vector<Scene>& scenes,
        const core::SceneId& currentId
    ) noexcept;

    static core::TimelineTime calculateProjectDuration(
        const std::vector<Scene>& scenes
    ) noexcept;
};

} // namespace catchim::editor
