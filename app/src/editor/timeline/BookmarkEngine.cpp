#include "editor/timeline/BookmarkEngine.h"
#include <algorithm>

namespace catchim::editor {

int BookmarkEngine::findBookmarkIndex(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime frameTime
) noexcept {
    for (size_t i = 0; i < bookmarks.size(); ++i) {
        if (bookmarks[i].time == frameTime) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool BookmarkEngine::isBookmarkAtTime(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime frameTime
) noexcept {
    return findBookmarkIndex(bookmarks, frameTime) != -1;
}

std::vector<Bookmark> BookmarkEngine::toggleBookmarkInArray(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime frameTime
) {
    int idx = findBookmarkIndex(bookmarks, frameTime);
    if (idx != -1) {
        std::vector<Bookmark> result;
        result.reserve(bookmarks.size() - 1);
        for (size_t i = 0; i < bookmarks.size(); ++i) {
            if (static_cast<int>(i) != idx) {
                result.push_back(bookmarks[i]);
            }
        }
        return result;
    }

    std::vector<Bookmark> result = bookmarks;
    Bookmark newBookmark;
    newBookmark.time = frameTime;
    result.push_back(std::move(newBookmark));
    std::sort(result.begin(), result.end(), [](const Bookmark& a, const Bookmark& b) {
        return a.time < b.time;
    });
    return result;
}

std::vector<Bookmark> BookmarkEngine::removeBookmarkFromArray(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime frameTime
) {
    std::vector<Bookmark> result;
    result.reserve(bookmarks.size());
    for (const auto& bm : bookmarks) {
        if (bm.time != frameTime) {
            result.push_back(bm);
        }
    }
    return result;
}

std::vector<Bookmark> BookmarkEngine::updateBookmarkInArray(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime frameTime,
    const std::optional<std::string>& note,
    const std::optional<std::string>& color,
    const std::optional<core::TimelineTime>& duration
) {
    int idx = findBookmarkIndex(bookmarks, frameTime);
    if (idx == -1) {
        return bookmarks;
    }

    std::vector<Bookmark> result = bookmarks;
    if (note.has_value()) {
        result[idx].note = *note;
    }
    if (color.has_value()) {
        result[idx].color = *color;
    }
    if (duration.has_value()) {
        result[idx].duration = *duration;
    }
    return result;
}

std::vector<Bookmark> BookmarkEngine::moveBookmarkInArray(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime fromTime,
    core::TimelineTime toTime
) {
    int idx = findBookmarkIndex(bookmarks, fromTime);
    if (idx == -1) {
        return bookmarks;
    }

    std::vector<Bookmark> result = bookmarks;
    result[idx].time = toTime;
    std::sort(result.begin(), result.end(), [](const Bookmark& a, const Bookmark& b) {
        return a.time < b.time;
    });
    return result;
}

const Bookmark* BookmarkEngine::getBookmarkAtTime(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime frameTime
) noexcept {
    int idx = findBookmarkIndex(bookmarks, frameTime);
    if (idx == -1) {
        return nullptr;
    }
    return &bookmarks[idx];
}

std::vector<Bookmark> BookmarkEngine::getBookmarksActiveAtTime(
    const std::vector<Bookmark>& bookmarks,
    core::TimelineTime time
) {
    std::vector<Bookmark> result;
    for (const auto& bm : bookmarks) {
        auto start = bm.time;
        auto end = (bm.duration.has_value() && bm.duration->ticks() > 0)
            ? start + *bm.duration
            : start;
        if (time >= start && time <= end) {
            result.push_back(bm);
        }
    }
    return result;
}

std::vector<BookmarkSnapPoint> BookmarkEngine::getBookmarkSnapPoints(
    const std::vector<Bookmark>& bookmarks,
    std::optional<core::TimelineTime> excludeBookmarkTime
) {
    std::vector<BookmarkSnapPoint> result;
    result.reserve(bookmarks.size());
    for (const auto& bm : bookmarks) {
        if (excludeBookmarkTime.has_value() && bm.time == *excludeBookmarkTime) {
            continue;
        }
        result.push_back(BookmarkSnapPoint{bm.time, "bookmark"});
    }
    return result;
}

} // namespace catchim::editor
