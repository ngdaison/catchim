#include "Project.h"

namespace catchim::editor {

Project::Project()
    : Project("New project")
{
}

Project::Project(std::string name)
{
    metadata_.id = core::ProjectId::generate();
    metadata_.name = std::move(name);

    // Initial scene
    core::SceneId mainSceneId("scene-1");
    scenes_.emplace_back(mainSceneId, "Main scene", true);
    currentSceneId_ = mainSceneId;
}

void Project::setName(std::string name) {
    if (metadata_.name != name) {
        metadata_.name = std::move(name);
        isDirty_ = true;
    }
}

Scene* Project::activeScene() {
    for (auto& s : scenes_) {
        if (s.id() == currentSceneId_) return &s;
    }
    if (!scenes_.empty()) return &scenes_[0];
    return nullptr;
}

const Scene* Project::activeScene() const {
    for (const auto& s : scenes_) {
        if (s.id() == currentSceneId_) return &s;
    }
    if (!scenes_.empty()) return &scenes_[0];
    return nullptr;
}

Scene* Project::findScene(const core::SceneId& sceneId) {
    for (auto& s : scenes_) {
        if (s.id() == sceneId) return &s;
    }
    return nullptr;
}

const Scene* Project::findScene(const core::SceneId& sceneId) const {
    for (const auto& s : scenes_) {
        if (s.id() == sceneId) return &s;
    }
    return nullptr;
}

Scene& Project::createScene(std::string name) {
    core::SceneId newId = core::SceneId::generate();
    scenes_.emplace_back(newId, std::move(name), false);
    isDirty_ = true;
    return scenes_.back();
}

bool Project::deleteScene(const core::SceneId& sceneId) {
    if (scenes_.size() <= 1) {
        return false; // Cannot delete last remaining scene
    }

    auto it = std::find_if(scenes_.begin(), scenes_.end(), [&](const Scene& s) {
        return s.id() == sceneId;
    });

    if (it != scenes_.end()) {
        bool wasActive = (it->id() == currentSceneId_);
        scenes_.erase(it);
        if (wasActive) {
            currentSceneId_ = scenes_.front().id();
        }
        isDirty_ = true;
        return true;
    }
    return false;
}

bool Project::switchScene(const core::SceneId& sceneId) {
    if (findScene(sceneId) != nullptr) {
        currentSceneId_ = sceneId;
        return true;
    }
    return false;
}

std::optional<core::SceneId> Project::duplicateScene(const core::SceneId& sceneId) {
    const Scene* src = findScene(sceneId);
    if (!src) return std::nullopt;

    core::SceneId dupId = core::SceneId::generate();
    Scene dupScene(dupId, src->name() + " (Copy)", false);
    dupScene.timeline() = src->timeline();

    scenes_.push_back(std::move(dupScene));
    isDirty_ = true;
    return dupId;
}

Timeline* Project::activeTimeline() {
    Scene* s = activeScene();
    return s ? &s->timeline() : nullptr;
}

const Timeline* Project::activeTimeline() const {
    const Scene* s = activeScene();
    return s ? &s->timeline() : nullptr;
}

core::TimelineTime Project::totalDuration() const {
    const Timeline* t = activeTimeline();
    return t ? t->totalDuration() : core::TimelineTime(0);
}

} // namespace catchim::editor
