#pragma once

#include "editor/timeline/Bookmark.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <optional>
#include <string>

namespace catchim::editor {

struct BookmarkSnapPoint {
    core::TimelineTime time{0};
    std::string type{"bookmark"};

    bool operator==(const BookmarkSnapPoint& other) const noexcept {
        return time == other.time && type == other.type;
    }
    bool operator!=(const BookmarkSnapPoint& other) const noexcept {
        return !(*this == other);
    }
};

class BookmarkEngine {
public:
    static int findBookmarkIndex(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime frameTime
    ) noexcept;

    static bool isBookmarkAtTime(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime frameTime
    ) noexcept;

    static std::vector<Bookmark> toggleBookmarkInArray(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime frameTime
    );

    static std::vector<Bookmark> removeBookmarkFromArray(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime frameTime
    );

    static std::vector<Bookmark> updateBookmarkInArray(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime frameTime,
        const std::optional<std::string>& note = std::nullopt,
        const std::optional<std::string>& color = std::nullopt,
        const std::optional<core::TimelineTime>& duration = std::nullopt
    );

    static std::vector<Bookmark> moveBookmarkInArray(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime fromTime,
        core::TimelineTime toTime
    );

    static const Bookmark* getBookmarkAtTime(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime frameTime
    ) noexcept;

    static std::vector<Bookmark> getBookmarksActiveAtTime(
        const std::vector<Bookmark>& bookmarks,
        core::TimelineTime time
    );

    static std::vector<BookmarkSnapPoint> getBookmarkSnapPoints(
        const std::vector<Bookmark>& bookmarks,
        std::optional<core::TimelineTime> excludeBookmarkTime = std::nullopt
    );
};

} // namespace catchim::editor
