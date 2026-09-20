#pragma once

#include "editor/project/Project.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>

namespace catchim::editor {

enum class ProjectSortKey {
    UpdatedAt,
    CreatedAt,
    Name,
    Duration
};

enum class SortDirection {
    Ascending,
    Descending
};

struct MigrationState {
    bool isMigrating{false};
    int fromVersion{0};
    int toVersion{0};
    std::string projectName{""};
};

class ProjectManager {
public:
    ProjectManager() = default;

    Project* getActive() noexcept { return activeProject_.get(); }
    const Project* getActive() const noexcept { return activeProject_.get(); }

    void setActive(std::unique_ptr<Project> project);
    void closeActive() noexcept;

    /**
     * @brief Creates a new project with initialized metadata, default canvas, FPS, background,
     * and a default main scene.
     * Mirrors web/src/core/managers/project-manager.ts createNewProject.
     */
    Project& createNewProject(std::string name);

    /**
     * @brief Calculates total project duration from scenes (duration of main scene or scenes[0]).
     * Mirrors web/src/timeline/scenes.ts getProjectDurationFromScenes.
     */
    static core::TimelineTime getProjectDurationFromScenes(const std::vector<Scene>& scenes);

    /**
     * @brief Sorts a list of ProjectMetadata by specified key and direction.
     * Mirrors web/src/core/managers/project-manager.ts sorting.
     */
    static void sortProjects(
        std::vector<ProjectMetadata>& list,
        ProjectSortKey key,
        SortDirection direction = SortDirection::Descending
    );

    // Saved projects catalogue
    const std::vector<ProjectMetadata>& savedProjects() const noexcept { return savedProjects_; }
    std::vector<ProjectMetadata>& savedProjects() noexcept { return savedProjects_; }
    void setSavedProjects(std::vector<ProjectMetadata> projects);

    // Migration state
    const MigrationState& migrationState() const noexcept { return migrationState_; }
    void setMigrationState(MigrationState state) noexcept;

    // Listeners & Dirty tracking
    bool isDirty() const noexcept;
    void setDirty(bool dirty = true) noexcept;

    void subscribe(std::function<void()> listener);
    void notify();

private:
    std::unique_ptr<Project> activeProject_{nullptr};
    std::vector<ProjectMetadata> savedProjects_;
    MigrationState migrationState_;
    std::vector<std::function<void()>> listeners_;
};

} // namespace catchim::editor
