#include "TimelineSceneUtils.h"
#include <algorithm>

namespace catchim::editor {

const Scene* TimelineSceneUtils::getMainScene(const std::vector<Scene>& scenes) noexcept {
    for (const auto& s : scenes) {
        if (s.isMain()) {
            return &s;
        }
    }
    return nullptr;
}

Scene* TimelineSceneUtils::getMainScene(std::vector<Scene>& scenes) noexcept {
    for (auto& s : scenes) {
        if (s.isMain()) {
            return &s;
        }
    }
    return nullptr;
}

void TimelineSceneUtils::ensureMainScene(std::vector<Scene>& scenes) {
    if (scenes.empty()) {
        scenes.emplace_back(core::SceneId::generate(), "Main scene", true);
        return;
    }

    bool hasMain = false;
    for (const auto& s : scenes) {
        if (s.isMain()) {
            hasMain = true;
            break;
        }
    }

    if (!hasMain) {
        scenes[0].setIsMain(true);
    }
}

bool TimelineSceneUtils::canDeleteScene(const Scene& scene) noexcept {
    return !scene.isMain();
}

const Scene* TimelineSceneUtils::getFallbackSceneAfterDelete(
    const std::vector<Scene>& scenes,
    const core::SceneId& deletedId,
    const core::SceneId& currentId
) noexcept {
    if (currentId != deletedId) {
        for (const auto& s : scenes) {
            if (s.id() == currentId) {
                return &s;
            }
        }
    }

    if (const auto* main = getMainScene(scenes)) {
        return main;
    }

    return scenes.empty() ? nullptr : &scenes[0];
}

const Scene* TimelineSceneUtils::findCurrentScene(
    const std::vector<Scene>& scenes,
    const core::SceneId& currentId
) noexcept {
    for (const auto& s : scenes) {
        if (s.id() == currentId) {
            return &s;
        }
    }

    if (const auto* main = getMainScene(scenes)) {
        return main;
    }

    return scenes.empty() ? nullptr : &scenes[0];
}

core::TimelineTime TimelineSceneUtils::calculateProjectDuration(
    const std::vector<Scene>& scenes
) noexcept {
    const Scene* target = getMainScene(scenes);
    if (!target && !scenes.empty()) {
        target = &scenes[0];
    }
    if (!target) {
        return core::TimelineTime::zero();
    }
    return target->timeline().totalDuration();
}

} // namespace catchim::editor
