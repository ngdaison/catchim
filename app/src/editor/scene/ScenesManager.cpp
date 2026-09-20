#include "editor/scene/ScenesManager.h"
#include "editor/history/commands/SceneCommands.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace catchim::editor {

ScenesManager::ScenesManager(Project& project)
    : project_(project)
{
    ensureMainScene();
    if (!project_.scenes().empty()) {
        if (project_.currentSceneId().isEmpty()) {
            project_.setCurrentSceneId(project_.scenes().front().id());
        }
        activeSceneId_ = project_.currentSceneId();
    }
}

Scene* ScenesManager::activeScene() noexcept {
    return project_.findScene(activeSceneId_);
}

const Scene* ScenesManager::activeScene() const noexcept {
    return project_.findScene(activeSceneId_);
}

void ScenesManager::ensureMainScene() {
    auto& scs = project_.scenes();
    if (scs.empty()) {
        auto mainId = core::SceneId::generate();
        Scene mainScene(mainId, "Main Scene", true);
        scs.push_back(std::move(mainScene));
        project_.setCurrentSceneId(mainId);
        activeSceneId_ = mainId;
        touchProjectTimestamp();
        notifyListeners();
        return;
    }

    bool hasMain = false;
    for (const auto& s : scs) {
        if (s.isMain()) {
            hasMain = true;
            break;
        }
    }

    if (!hasMain) {
        scs.front().setIsMain(true);
        touchProjectTimestamp();
        notifyListeners();
    }
}

core::SceneId ScenesManager::createScene(std::string name, bool isMain) {
    auto newId = core::SceneId::generate();

    if (isMain) {
        for (auto& s : project_.scenes()) {
            s.setIsMain(false);
        }
    }

    Scene scene(newId, std::move(name), isMain);
    project_.scenes().push_back(std::move(scene));
    activeSceneId_ = newId;
    project_.setCurrentSceneId(newId);

    touchProjectTimestamp();
    notifyListeners();
    return newId;
}

bool ScenesManager::renameScene(const core::SceneId& sceneId, std::string newName) {
    auto* s = project_.findScene(sceneId);
    if (!s) return false;

    s->setName(std::move(newName));
    touchProjectTimestamp();
    notifyListeners();
    return true;
}

SceneDeletionResult ScenesManager::canDeleteScene(const core::SceneId& sceneId) const noexcept {
    const auto* s = project_.findScene(sceneId);
    if (!s) {
        return SceneDeletionResult{false, "Scene not found"};
    }
    if (project_.scenes().size() <= 1) {
        return SceneDeletionResult{false, "Cannot delete the only scene in the project"};
    }
    if (s->isMain()) {
        return SceneDeletionResult{false, "Cannot delete the main scene"};
    }
    return SceneDeletionResult{true, ""};
}

bool ScenesManager::deleteScene(const core::SceneId& sceneId) {
    auto check = canDeleteScene(sceneId);
    if (!check.canDelete) {
        return false;
    }

    auto fallbackId = SceneUtils::getFallbackSceneAfterDelete(
        project_.scenes(),
        sceneId,
        activeSceneId_
    );

    auto& scs = project_.scenes();
    scs.erase(
        std::remove_if(scs.begin(), scs.end(), [&](const Scene& candidate) {
            return candidate.id() == sceneId;
        }),
        scs.end()
    );

    if (!fallbackId.isEmpty()) {
        activeSceneId_ = fallbackId;
        project_.setCurrentSceneId(fallbackId);
    } else if (!scs.empty()) {
        activeSceneId_ = scs.front().id();
        project_.setCurrentSceneId(activeSceneId_);
    }

    touchProjectTimestamp();
    notifyListeners();
    return true;
}

bool ScenesManager::switchToScene(const core::SceneId& sceneId) {
    if (!project_.switchScene(sceneId)) {
        return false;
    }
    activeSceneId_ = sceneId;
    touchProjectTimestamp();
    notifyListeners();
    return true;
}

bool ScenesManager::isBookmarkedAtTime(
    core::TimelineTime time,
    const core::FrameRate& fps
) const noexcept {
    return getBookmarkAtTime(time, fps) != nullptr;
}

const Bookmark* ScenesManager::getBookmarkAtTime(
    core::TimelineTime time,
    const core::FrameRate& fps
) const noexcept {
    const auto* curScene = activeScene();
    if (!curScene) return nullptr;

    int64_t frameTicks = core::RationalFrameRateHelper::roundFrameTicks(time.ticks(), fps);
    core::TimelineTime targetTime = core::TimelineTime::fromTicks(frameTicks);

    for (const auto& b : curScene->timeline().bookmarks()) {
        int64_t bTicks = core::RationalFrameRateHelper::roundFrameTicks(b.time.ticks(), fps);
        if (bTicks == targetTime.ticks()) {
            return &b;
        }
    }
    return nullptr;
}

bool ScenesManager::toggleBookmark(
    core::TimelineTime time,
    const std::string& note,
    const std::string& color,
    const core::FrameRate& fps
) {
    auto* curScene = activeScene();
    if (!curScene) return false;

    int64_t frameTicks = core::RationalFrameRateHelper::roundFrameTicks(time.ticks(), fps);
    core::TimelineTime targetTime = core::TimelineTime::fromTicks(frameTicks);

    auto& bks = curScene->timeline().bookmarks();
    auto it = std::find_if(bks.begin(), bks.end(), [&](const Bookmark& b) {
        int64_t bTicks = core::RationalFrameRateHelper::roundFrameTicks(b.time.ticks(), fps);
        return bTicks == targetTime.ticks();
    });

    if (it != bks.end()) {
        // Remove existing
        bks.erase(it);
    } else {
        // Add new
        Bookmark b;
        b.id = core::BookmarkId::generate();
        b.time = targetTime;
        b.note = note;
        b.color = color;
        bks.push_back(std::move(b));
        std::sort(bks.begin(), bks.end());
    }

    touchProjectTimestamp();
    notifyListeners();
    return true;
}

bool ScenesManager::removeBookmark(
    core::TimelineTime time,
    const core::FrameRate& fps
) {
    auto* curScene = activeScene();
    if (!curScene) return false;

    int64_t frameTicks = core::RationalFrameRateHelper::roundFrameTicks(time.ticks(), fps);
    core::TimelineTime targetTime = core::TimelineTime::fromTicks(frameTicks);

    auto& bks = curScene->timeline().bookmarks();
    auto it = std::find_if(bks.begin(), bks.end(), [&](const Bookmark& b) {
        int64_t bTicks = core::RationalFrameRateHelper::roundFrameTicks(b.time.ticks(), fps);
        return bTicks == targetTime.ticks();
    });

    if (it != bks.end()) {
        bks.erase(it);
        touchProjectTimestamp();
        notifyListeners();
        return true;
    }
    return false;
}

void ScenesManager::addListener(SceneChangeListener listener) {
    if (listener) {
        listeners_.push_back(std::move(listener));
    }
}

void ScenesManager::notifyListeners() noexcept {
    for (auto& l : listeners_) {
        if (l) l();
    }
}

void ScenesManager::touchProjectTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ");
    project_.metadata().updatedAt = ss.str();
    project_.setDirty(true);
}

} // namespace catchim::editor
