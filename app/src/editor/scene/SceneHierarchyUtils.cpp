#include "SceneHierarchyUtils.h"
#include <algorithm>

namespace catchim::editor {

const Scene* SceneHierarchyUtils::getMainScene(const std::vector<Scene>& scenes) noexcept {
    for (const auto& scene : scenes) {
        if (scene.isMain()) {
            return &scene;
        }
    }
    return nullptr;
}

Scene* SceneHierarchyUtils::getMainScene(std::vector<Scene>& scenes) noexcept {
    for (auto& scene : scenes) {
        if (scene.isMain()) {
            return &scene;
        }
    }
    return nullptr;
}

void SceneHierarchyUtils::ensureMainScene(std::vector<Scene>& scenes) {
    if (getMainScene(scenes) != nullptr) {
        return;
    }
    scenes.insert(scenes.begin(), buildDefaultScene("Main scene", true));
}

Scene SceneHierarchyUtils::buildDefaultScene(
    const std::string& name,
    bool isMain
) {
    return Scene(core::SceneId::generate(), name, isMain);
}

SceneDeletionResult SceneHierarchyUtils::canDeleteScene(const Scene& scene) noexcept {
    if (scene.isMain()) {
        return SceneDeletionResult{false, "Cannot delete main scene"};
    }
    return SceneDeletionResult{true, ""};
}

const Scene* SceneHierarchyUtils::getFallbackSceneAfterDelete(
    const std::vector<Scene>& scenes,
    const core::SceneId& deletedSceneId,
    const std::optional<core::SceneId>& currentSceneId
) noexcept {
    if (currentSceneId.has_value() && *currentSceneId != deletedSceneId) {
        for (const auto& scene : scenes) {
            if (scene.id() == *currentSceneId) {
                return &scene;
            }
        }
    }
    return getMainScene(scenes);
}

const Scene* SceneHierarchyUtils::findCurrentScene(
    const std::vector<Scene>& scenes,
    const std::optional<core::SceneId>& currentSceneId
) noexcept {
    if (currentSceneId.has_value()) {
        for (const auto& scene : scenes) {
            if (scene.id() == *currentSceneId) {
                return &scene;
            }
        }
    }

    const auto* mainScene = getMainScene(scenes);
    if (mainScene != nullptr) {
        return mainScene;
    }

    if (!scenes.empty()) {
        return &scenes.front();
    }

    return nullptr;
}

core::TimelineTime SceneHierarchyUtils::calculateTotalDuration(const Timeline& timeline) noexcept {
    core::TimelineTime maxEnd{0};
    for (const auto* track : timeline.allTracks()) {
        if (!track) continue;
        for (const auto& clip : track->clips()) {
            const auto clipEnd = clip.startTime() + clip.duration();
            if (clipEnd > maxEnd) {
                maxEnd = clipEnd;
            }
        }
    }
    return maxEnd;
}

core::TimelineTime SceneHierarchyUtils::getProjectDurationFromScenes(
    const std::vector<Scene>& scenes
) noexcept {
    const auto* mainScene = getMainScene(scenes);
    if (mainScene == nullptr && !scenes.empty()) {
        mainScene = &scenes.front();
    }
    if (mainScene == nullptr) {
        return core::TimelineTime{0};
    }
    return calculateTotalDuration(mainScene->timeline());
}

} // namespace catchim::editor
