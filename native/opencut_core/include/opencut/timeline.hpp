#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace opencut {

using TimelineTick = std::int64_t;

struct TimeRange {
    TimelineTick start = 0;
    TimelineTick duration = 0;

    [[nodiscard]] TimelineTick end() const noexcept;
    [[nodiscard]] bool is_valid() const noexcept;
    [[nodiscard]] bool overlaps(const TimeRange& other) const noexcept;
};

struct Clip {
    std::string id;
    TimeRange range;
};

struct Track {
    std::string id;
    std::vector<Clip> clips;
};

struct ClipRef {
    std::string_view track_id;
    std::string_view clip_id;
};

struct IndexedClip {
    std::string_view track_id;
    std::string_view clip_id;
    TimeRange range;
};

enum class TimelineErrorCode {
    None,
    InvalidTimeRange,
    MissingTrack,
    MissingClip,
    DuplicateTrack,
    DuplicateClip,
    Overlap,
};

struct TimelineError {
    TimelineErrorCode code = TimelineErrorCode::None;
    std::string message;
};

template <typename TValue>
struct TimelineResult {
    TValue value{};
    TimelineError error{};

    [[nodiscard]] bool ok() const noexcept
    {
        return error.code == TimelineErrorCode::None;
    }
};

template <>
struct TimelineResult<void> {
    TimelineError error{};

    [[nodiscard]] bool ok() const noexcept
    {
        return error.code == TimelineErrorCode::None;
    }
};

enum class SnapSourceType {
    Playhead,
    ClipStart,
    ClipEnd,
    Marker,
};

struct SnapPoint {
    TimelineTick time = 0;
    SnapSourceType type = SnapSourceType::Playhead;
    std::string label;
};

struct SnapResult {
    TimelineTick snapped_time = 0;
    TimelineTick delta = 0;
    bool snapped = false;
    std::optional<SnapPoint> matched_point = std::nullopt;
};

struct GroupMoveItem {
    std::string clip_id;
    std::string source_track_id;
    std::string target_track_id;
    TimelineTick new_start = 0;
    TimelineTick duration = 0;
};

struct GroupMoveResult {
    bool can_move = false;
    std::vector<std::string> conflicting_clip_ids;
};

class TimelineIndex {
public:
    TimelineIndex() = default;
    explicit TimelineIndex(std::vector<Track> tracks);

    [[nodiscard]] std::span<const Track> tracks() const noexcept;
    [[nodiscard]] const Track* find_track(std::string_view track_id) const noexcept;
    [[nodiscard]] const Clip* find_clip(ClipRef ref) const noexcept;

    [[nodiscard]] bool can_place(std::string_view track_id,
                                 TimeRange range,
                                 std::optional<std::string_view> exclude_clip_id = std::nullopt) const;

    [[nodiscard]] std::vector<IndexedClip> clips_in_range(TimeRange range) const;

    [[nodiscard]] TimelineResult<void> add_track(std::string track_id);

    [[nodiscard]] TimelineResult<void> insert_clip(std::string_view track_id, Clip clip);

    [[nodiscard]] TimelineResult<Clip> delete_clip(ClipRef ref);

    [[nodiscard]] TimelineResult<void> move_clip(ClipRef ref,
                                                 std::string_view target_track_id,
                                                 TimelineTick new_start);

    [[nodiscard]] TimelineResult<void> trim_clip(ClipRef ref,
                                                 TimelineTick new_start,
                                                 TimelineTick new_duration);

    [[nodiscard]] TimelineResult<Clip> split_clip(ClipRef ref, TimelineTick split_time, std::string new_clip_id);

    // Advanced snapping & group move methods
    [[nodiscard]] std::vector<SnapPoint> collect_snap_points(
        std::optional<TimelineTick> playhead = std::nullopt,
        const std::vector<TimelineTick>& markers = {}) const;

    [[nodiscard]] SnapResult snap_time(
        TimelineTick target_time,
        std::span<const SnapPoint> points,
        TimelineTick threshold_ticks) const;

    [[nodiscard]] GroupMoveResult check_group_move(
        std::span<const GroupMoveItem> items) const;

    [[nodiscard]] std::optional<TimelineTick> find_first_available_gap(
        std::string_view track_id,
        TimelineTick duration,
        TimelineTick min_start_time = 0) const;

    [[nodiscard]] std::vector<std::pair<std::string, TimelineTick>> apply_ripple_shift(
        std::string_view track_id,
        TimelineTick after_time,
        TimelineTick delta_ticks);

private:
    std::vector<Track> tracks_;
    std::unordered_map<std::string_view, std::size_t> track_by_id_;
    std::unordered_map<std::string_view, std::pair<std::size_t, std::size_t>> clip_by_id_;
    std::vector<std::vector<TimelineTick>> max_clip_end_by_track_;

    void rebuild_index();
    void sort_tracks();
    [[nodiscard]] Track* find_track_mut(std::string_view track_id) noexcept;
    [[nodiscard]] Clip* find_clip_mut(ClipRef ref) noexcept;
};

} // namespace opencut
