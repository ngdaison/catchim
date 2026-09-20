#include "editor/history/commands/ProjectCommands.h"
#include <algorithm>

namespace catchim::editor {

// ==========================================
// UpdateProjectSettingsCommand
// ==========================================

UpdateProjectSettingsCommand::UpdateProjectSettingsCommand(
    Project& project,
    ProjectSettings newSettings
)
    : project_(project),
      newSettings_(std::move(newSettings)) {}

bool UpdateProjectSettingsCommand::execute() {
    savedSettings_ = project_.settings();
    project_.settings() = newSettings_;
    return true;
}

bool UpdateProjectSettingsCommand::undo() {
    if (!savedSettings_) return false;
    project_.settings() = *savedSettings_;
    return true;
}

// ==========================================
// ToggleBookmarkCommand
// ==========================================

ToggleBookmarkCommand::ToggleBookmarkCommand(
    Timeline& timeline,
    core::TimelineTime time,
    std::string note,
    std::string color
)
    : timeline_(timeline),
      time_(time),
      note_(std::move(note)),
      color_(std::move(color)) {}

bool ToggleBookmarkCommand::execute() {
    savedBookmarks_ = timeline_.bookmarks();
    executed_ = true;

    constexpr core::TimelineTime threshold = core::TimelineTime::fromTicks(1000);
    bool exists = timeline_.hasBookmarkAt(time_, threshold);

    if (exists) {
        auto& bms = timeline_.bookmarks();
        bms.erase(std::remove_if(bms.begin(), bms.end(), [&](const Bookmark& b) {
            return std::abs(b.time.ticks() - time_.ticks()) <= threshold.ticks();
        }), bms.end());
    } else {
        Bookmark bm;
        bm.time = time_;
        bm.note = note_;
        bm.color = color_;
        timeline_.addBookmark(std::move(bm));
        timeline_.sortBookmarks();
    }

    return true;
}

bool ToggleBookmarkCommand::undo() {
    if (!executed_) return false;
    timeline_.setBookmarks(savedBookmarks_);
    return true;
}

// ==========================================
// MoveBookmarkCommand
// ==========================================

MoveBookmarkCommand::MoveBookmarkCommand(
    Timeline& timeline,
    core::BookmarkId bookmarkId,
    core::TimelineTime newTime
)
    : timeline_(timeline),
      bookmarkId_(std::move(bookmarkId)),
      newTime_(newTime) {}

bool MoveBookmarkCommand::execute() {
    savedBookmarks_ = timeline_.bookmarks();
    executed_ = true;

    for (auto& bm : timeline_.bookmarks()) {
        if (bm.id == bookmarkId_) {
            bm.time = newTime_;
            timeline_.sortBookmarks();
            return true;
        }
    }
    return false;
}

bool MoveBookmarkCommand::undo() {
    if (!executed_) return false;
    timeline_.setBookmarks(savedBookmarks_);
    return true;
}

// ==========================================
// UpdateBookmarkCommand
// ==========================================

UpdateBookmarkCommand::UpdateBookmarkCommand(
    Timeline& timeline,
    core::BookmarkId bookmarkId,
    std::string newNote,
    std::string newColor
)
    : timeline_(timeline),
      bookmarkId_(std::move(bookmarkId)),
      newNote_(std::move(newNote)),
      newColor_(std::move(newColor)) {}

bool UpdateBookmarkCommand::execute() {
    savedBookmarks_ = timeline_.bookmarks();
    executed_ = true;

    for (auto& bm : timeline_.bookmarks()) {
        if (bm.id == bookmarkId_) {
            bm.note = newNote_;
            bm.color = newColor_;
            return true;
        }
    }
    return false;
}

bool UpdateBookmarkCommand::undo() {
    if (!executed_) return false;
    timeline_.setBookmarks(savedBookmarks_);
    return true;
}

// ==========================================
// RemoveBookmarkCommand
// ==========================================

RemoveBookmarkCommand::RemoveBookmarkCommand(
    Timeline& timeline,
    core::BookmarkId bookmarkId
)
    : timeline_(timeline),
      bookmarkId_(std::move(bookmarkId)) {}

bool RemoveBookmarkCommand::execute() {
    savedBookmarks_ = timeline_.bookmarks();
    executed_ = true;
    return timeline_.removeBookmark(bookmarkId_);
}

bool RemoveBookmarkCommand::undo() {
    if (!executed_) return false;
    timeline_.setBookmarks(savedBookmarks_);
    return true;
}

} // namespace catchim::editor
