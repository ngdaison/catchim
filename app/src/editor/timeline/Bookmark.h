#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <optional>

namespace catchim::editor {

struct Bookmark {
    core::BookmarkId id{core::BookmarkId::generate()};
    core::TimelineTime time{0};
    std::string note;
    std::string color{"#009dff"};
    std::optional<core::TimelineTime> duration{std::nullopt};

    bool operator==(const Bookmark& other) const {
        return id == other.id;
    }
    bool operator<(const Bookmark& other) const {
        return time < other.time;
    }
};

} // namespace catchim::editor
