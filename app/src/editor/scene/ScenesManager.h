#pragma once

#include "editor/project/Project.h"
#include "editor/timeline/Bookmark.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "core/time/RationalFrameRate.h"
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace catchim::editor {

struct SceneDeletionResult {
    bool canDelete{false};
    std::string reason;
};

class ScenesManager {
public:
    using SceneChangeListener = std::function<void()>;

    explicit ScenesManager(Project& project);

    Project& project() noexcept { return project_; }
    const Project& project() const noexcept { return project_; }

    const core::SceneId& activeSceneId() const noexcept { return activeSceneId_; }
    Scene* activeScene() noexcept;
    const Scene* activeScene() const noexcept;

    // Invariant: ensures project has at least 1 scene and exactly 1 main scene
    void ensureMainScene();

    // Scene CRUD & switching
    core::SceneId createScene(std::string name, bool isMain = false);
    bool renameScene(const core::SceneId& sceneId, std::string newName);

    SceneDeletionResult canDeleteScene(const core::SceneId& sceneId) const noexcept;
    bool deleteScene(const core::SceneId& sceneId);

    bool switchToScene(const core::SceneId& sceneId);

    // Active Scene Bookmark Management
    bool isBookmarkedAtTime(
        core::TimelineTime time,
        const core::FrameRate& fps = core::FrameRate{30, 1}
    ) const noexcept;

    const Bookmark* getBookmarkAtTime(
        core::TimelineTime time,
        const core::FrameRate& fps = core::FrameRate{30, 1}
    ) const noexcept;

    bool toggleBookmark(
        core::TimelineTime time,
        const std::string& note = "",
        const std::string& color = "#009dff",
        const core::FrameRate& fps = core::FrameRate{30, 1}
    );

    bool removeBookmark(
        core::TimelineTime time,
        const core::FrameRate& fps = core::FrameRate{30, 1}
    );

    // Observers
    void addListener(SceneChangeListener listener);

private:
    void notifyListeners() noexcept;
    void touchProjectTimestamp();

    Project& project_;
    core::SceneId activeSceneId_;
    std::vector<SceneChangeListener> listeners_;
};

} // namespace catchim::editor
