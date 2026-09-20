#pragma once

#include "editor/history/Command.h"
#include "editor/project/Project.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/Bookmark.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::editor {

/**
 * @brief Command to update project settings (CanvasSize, FPS, Background, etc.).
 * Corresponds to web/src/commands/project/update-project-settings.ts.
 */
class UpdateProjectSettingsCommand : public EditorCommand {
public:
    UpdateProjectSettingsCommand(Project& project, ProjectSettings newSettings);

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Update Project Settings"; }

private:
    Project& project_;
    ProjectSettings newSettings_;
    std::optional<ProjectSettings> savedSettings_{std::nullopt};
};

/**
 * @brief Command to toggle a bookmark at a specific timeline timestamp.
 * Corresponds to web/src/commands/scene/toggle-bookmark.ts.
 */
class ToggleBookmarkCommand : public EditorCommand {
public:
    ToggleBookmarkCommand(
        Timeline& timeline,
        core::TimelineTime time,
        std::string note = "",
        std::string color = "#009dff"
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Toggle Bookmark"; }

private:
    Timeline& timeline_;
    core::TimelineTime time_;
    std::string note_;
    std::string color_;
    std::vector<Bookmark> savedBookmarks_;
    bool executed_{false};
};

/**
 * @brief Command to move an existing bookmark to a new timeline timestamp.
 * Corresponds to web/src/commands/scene/move-bookmark.ts.
 */
class MoveBookmarkCommand : public EditorCommand {
public:
    MoveBookmarkCommand(
        Timeline& timeline,
        core::BookmarkId bookmarkId,
        core::TimelineTime newTime
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Move Bookmark"; }

private:
    Timeline& timeline_;
    core::BookmarkId bookmarkId_;
    core::TimelineTime newTime_;
    std::vector<Bookmark> savedBookmarks_;
    bool executed_{false};
};

/**
 * @brief Command to update bookmark metadata (label note and highlight color).
 * Corresponds to web/src/commands/scene/update-bookmark.ts.
 */
class UpdateBookmarkCommand : public EditorCommand {
public:
    UpdateBookmarkCommand(
        Timeline& timeline,
        core::BookmarkId bookmarkId,
        std::string newNote,
        std::string newColor
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Update Bookmark"; }

private:
    Timeline& timeline_;
    core::BookmarkId bookmarkId_;
    std::string newNote_;
    std::string newColor_;
    std::vector<Bookmark> savedBookmarks_;
    bool executed_{false};
};

/**
 * @brief Command to remove a bookmark by ID.
 * Corresponds to web/src/commands/scene/remove-bookmark.ts.
 */
class RemoveBookmarkCommand : public EditorCommand {
public:
    RemoveBookmarkCommand(
        Timeline& timeline,
        core::BookmarkId bookmarkId
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Remove Bookmark"; }

private:
    Timeline& timeline_;
    core::BookmarkId bookmarkId_;
    std::vector<Bookmark> savedBookmarks_;
    bool executed_{false};
};

} // namespace catchim::editor
