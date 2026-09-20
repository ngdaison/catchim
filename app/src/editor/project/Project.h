#pragma once

#include "ProjectSettings.h"
#include "editor/timeline/Timeline.h"
#include "core/ids/Ids.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace catchim::editor {

struct ProjectMetadata {
    core::ProjectId id;
    std::string name{"New project"};
    std::string thumbnail{""};
    core::TimelineTime duration{core::TimelineTime(0)};
    std::string createdAt{""};
    std::string updatedAt{""};
};

struct TimelineViewState {
    double zoomLevel{1.0};
    double scrollLeft{0.0};
    core::TimelineTime playheadTime{core::TimelineTime(0)};
};

class Scene {
public:
    Scene(core::SceneId id, std::string name, bool isMain = false)
        : id_(std::move(id)), name_(std::move(name)), isMain_(isMain) {}

    const core::SceneId& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    void setName(std::string name) { name_ = std::move(name); }
    bool isMain() const noexcept { return isMain_; }
    void setIsMain(bool isMain) noexcept { isMain_ = isMain; }

    const Timeline& timeline() const noexcept { return timeline_; }
    Timeline& timeline() noexcept { return timeline_; }

private:
    core::SceneId id_;
    std::string name_;
    bool isMain_{false};
    Timeline timeline_;
};

class Project {
public:
    Project();
    explicit Project(std::string name);

    const ProjectMetadata& metadata() const noexcept { return metadata_; }
    ProjectMetadata& metadata() noexcept { return metadata_; }

    const std::string& name() const noexcept { return metadata_.name; }
    void setName(std::string name);

    const ProjectSettings& settings() const noexcept { return settings_; }
    ProjectSettings& settings() noexcept { return settings_; }

    int version() const noexcept { return version_; }

    // Scenes
    const std::vector<Scene>& scenes() const noexcept { return scenes_; }
    std::vector<Scene>& scenes() noexcept { return scenes_; }

    const core::SceneId& currentSceneId() const noexcept { return currentSceneId_; }
    void setCurrentSceneId(core::SceneId id) { currentSceneId_ = std::move(id); }

    Scene* activeScene();
    const Scene* activeScene() const;

    Scene* findScene(const core::SceneId& sceneId);
    const Scene* findScene(const core::SceneId& sceneId) const;

    Scene& createScene(std::string name);
    bool deleteScene(const core::SceneId& sceneId);
    bool switchScene(const core::SceneId& sceneId);
    std::optional<core::SceneId> duplicateScene(const core::SceneId& sceneId);

    Timeline* activeTimeline();
    const Timeline* activeTimeline() const;

    // View state
    const TimelineViewState& viewState() const noexcept { return viewState_; }
    TimelineViewState& viewState() noexcept { return viewState_; }

    // Dirty state
    bool isDirty() const noexcept { return isDirty_; }
    void setDirty(bool dirty = true) noexcept { isDirty_ = dirty; }

    core::TimelineTime totalDuration() const;

private:
    ProjectMetadata metadata_;
    ProjectSettings settings_;
    int version_{31};
    std::vector<Scene> scenes_;
    core::SceneId currentSceneId_;
    TimelineViewState viewState_;
    bool isDirty_{false};
};

} // namespace catchim::editor
