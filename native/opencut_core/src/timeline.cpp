#include "opencut/timeline.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <utility>

namespace opencut {

namespace {

TimelineError make_error(TimelineErrorCode code, std::string message)
{
    return TimelineError{.code = code, .message = std::move(message)};
}

bool same_id(std::string_view left, std::string_view right) noexcept
{
    return left == right;
}

bool clip_id_exists(const std::unordered_map<std::string_view, std::pair<std::size_t, std::size_t>>& index,
                    std::string_view clip_id)
{
    return index.find(clip_id) != index.end();
}

} // namespace

TimelineTick TimeRange::end() const noexcept
{
    return start + duration;
}

bool TimeRange::is_valid() const noexcept
{
    if (start < 0 || duration < 0) {
        return false;
    }

    return start <= std::numeric_limits<TimelineTick>::max() - duration;
}

bool TimeRange::overlaps(const TimeRange& other) const noexcept
{
    return start < other.end() && end() > other.start;
}

TimelineIndex::TimelineIndex(std::vector<Track> tracks)
    : tracks_(std::move(tracks))
{
    sort_tracks();
    rebuild_index();
}

std::span<const Track> TimelineIndex::tracks() const noexcept
{
    return tracks_;
}

const Track* TimelineIndex::find_track(std::string_view track_id) const noexcept
{
    const auto found = track_by_id_.find(track_id);
    if (found == track_by_id_.end()) {
        return nullptr;
    }

    return &tracks_[found->second];
}

Track* TimelineIndex::find_track_mut(std::string_view track_id) noexcept
{
    const auto found = track_by_id_.find(track_id);
    if (found == track_by_id_.end()) {
        return nullptr;
    }

    return &tracks_[found->second];
}

const Clip* TimelineIndex::find_clip(ClipRef ref) const noexcept
{
    const auto found = clip_by_id_.find(ref.clip_id);
    if (found == clip_by_id_.end()) {
        return nullptr;
    }

    const auto [track_index, clip_index] = found->second;
    const Track& track = tracks_[track_index];
    if (!same_id(track.id, ref.track_id)) {
        return nullptr;
    }

    return &track.clips[clip_index];
}

Clip* TimelineIndex::find_clip_mut(ClipRef ref) noexcept
{
    const auto found = clip_by_id_.find(ref.clip_id);
    if (found == clip_by_id_.end()) {
        return nullptr;
    }

    const auto [track_index, clip_index] = found->second;
    Track& track = tracks_[track_index];
    if (!same_id(track.id, ref.track_id)) {
        return nullptr;
    }

    return &track.clips[clip_index];
}

bool TimelineIndex::can_place(std::string_view track_id,
                              TimeRange range,
                              std::optional<std::string_view> exclude_clip_id) const
{
    if (!range.is_valid()) {
        return false;
    }

    const Track* track = find_track(track_id);
    if (track == nullptr) {
        return false;
    }

    const auto first_candidate = std::lower_bound(
        track->clips.begin(),
        track->clips.end(),
        range.start,
        [](const Clip& clip, TimelineTick start) {
            return clip.range.start < start;
        });

    for (auto it = first_candidate; it != track->clips.end(); ++it) {
        if (it->range.start >= range.end()) {
            break;
        }
        if (exclude_clip_id && same_id(it->id, *exclude_clip_id)) {
            continue;
        }
        if (it->range.overlaps(range)) {
            return false;
        }
    }

    for (auto it = first_candidate; it != track->clips.begin();) {
        --it;
        const std::size_t index = static_cast<std::size_t>(std::distance(track->clips.begin(), it));
        if (max_clip_end_by_track_[track_by_id_.at(track_id)][index] <= range.start) {
            break;
        }
        if (exclude_clip_id && same_id(it->id, *exclude_clip_id)) {
            continue;
        }
        if (it->range.overlaps(range)) {
            return false;
        }
    }

    return true;
}

std::vector<IndexedClip> TimelineIndex::clips_in_range(TimeRange range) const
{
    std::vector<IndexedClip> result;
    if (!range.is_valid()) {
        return result;
    }

    for (const Track& track : tracks_) {
        const auto first_candidate = std::lower_bound(
            track.clips.begin(),
            track.clips.end(),
            range.start,
            [](const Clip& clip, TimelineTick start) {
                return clip.range.start < start;
            });

        for (auto it = first_candidate; it != track.clips.end(); ++it) {
            if (it->range.start >= range.end()) {
                break;
            }
            if (it->range.overlaps(range)) {
                result.push_back(IndexedClip{
                    .track_id = track.id,
                    .clip_id = it->id,
                    .range = it->range,
                });
            }
        }

        for (auto it = first_candidate; it != track.clips.begin();) {
            --it;
            const std::size_t index = static_cast<std::size_t>(std::distance(track.clips.begin(), it));
            if (max_clip_end_by_track_[track_by_id_.at(track.id)][index] <= range.start) {
                break;
            }
            if (it->range.overlaps(range)) {
                result.push_back(IndexedClip{
                    .track_id = track.id,
                    .clip_id = it->id,
                    .range = it->range,
                });
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const IndexedClip& left, const IndexedClip& right) {
        if (left.range.start != right.range.start) {
            return left.range.start < right.range.start;
        }
        return left.clip_id < right.clip_id;
    });

    return result;
}

TimelineResult<void> TimelineIndex::add_track(std::string track_id)
{
    if (track_id.empty() || track_by_id_.find(track_id) != track_by_id_.end()) {
        return {.error = make_error(TimelineErrorCode::DuplicateTrack, "track id already exists or is empty")};
    }

    tracks_.push_back(Track{.id = std::move(track_id), .clips = {}});
    rebuild_index();
    return {};
}

TimelineResult<void> TimelineIndex::insert_clip(std::string_view track_id, Clip clip)
{
    if (!clip.range.is_valid()) {
        return {.error = make_error(TimelineErrorCode::InvalidTimeRange, "invalid clip range")};
    }

    if (clip.id.empty() || clip_id_exists(clip_by_id_, clip.id)) {
        return {.error = make_error(TimelineErrorCode::DuplicateClip, "clip id already exists")};
    }

    Track* track = find_track_mut(track_id);
    if (track == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingTrack, "target track not found")};
    }

    if (!can_place(track_id, clip.range)) {
        return {.error = make_error(TimelineErrorCode::Overlap, "clip range overlaps another clip")};
    }

    track->clips.push_back(std::move(clip));
    sort_tracks();
    rebuild_index();
    return {};
}

TimelineResult<Clip> TimelineIndex::delete_clip(ClipRef ref)
{
    Track* track = find_track_mut(ref.track_id);
    if (track == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingTrack, "track not found")};
    }

    const auto found = std::find_if(track->clips.begin(), track->clips.end(), [&](const Clip& clip) {
        return same_id(clip.id, ref.clip_id);
    });
    if (found == track->clips.end()) {
        return {.error = make_error(TimelineErrorCode::MissingClip, "clip not found")};
    }

    Clip removed = std::move(*found);
    track->clips.erase(found);
    rebuild_index();
    return {.value = std::move(removed)};
}

TimelineResult<void> TimelineIndex::move_clip(ClipRef ref,
                                              std::string_view target_track_id,
                                              TimelineTick new_start)
{
    const Clip* source_clip = find_clip(ref);
    if (source_clip == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingClip, "source clip not found")};
    }

    const TimeRange next_range{.start = new_start, .duration = source_clip->range.duration};
    if (!next_range.is_valid()) {
        return {.error = make_error(TimelineErrorCode::InvalidTimeRange, "invalid target range")};
    }

    if (find_track(target_track_id) == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingTrack, "target track not found")};
    }

    if (!can_place(target_track_id, next_range, ref.clip_id)) {
        return {.error = make_error(TimelineErrorCode::Overlap, "target range overlaps another clip")};
    }

    Clip moved_clip = *source_clip;
    moved_clip.range = next_range;

    Track* source_track = find_track_mut(ref.track_id);
    Track* target_track = find_track_mut(target_track_id);
    if (source_track == nullptr || target_track == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingTrack, "track not found")};
    }

    std::erase_if(source_track->clips, [&](const Clip& clip) {
        return same_id(clip.id, ref.clip_id);
    });
    target_track->clips.push_back(std::move(moved_clip));

    sort_tracks();
    rebuild_index();
    return {};
}

TimelineResult<void> TimelineIndex::trim_clip(ClipRef ref,
                                              TimelineTick new_start,
                                              TimelineTick new_duration)
{
    const Clip* clip = find_clip(ref);
    if (clip == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingClip, "clip not found")};
    }

    const TimeRange next_range{.start = new_start, .duration = new_duration};
    if (!next_range.is_valid()) {
        return {.error = make_error(TimelineErrorCode::InvalidTimeRange, "invalid trim range")};
    }

    if (!can_place(ref.track_id, next_range, ref.clip_id)) {
        return {.error = make_error(TimelineErrorCode::Overlap, "trim range overlaps another clip")};
    }

    Clip* mutable_clip = find_clip_mut(ref);
    if (mutable_clip == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingClip, "clip not found")};
    }

    mutable_clip->range = next_range;
    sort_tracks();
    rebuild_index();
    return {};
}

TimelineResult<Clip> TimelineIndex::split_clip(ClipRef ref, TimelineTick split_time, std::string new_clip_id)
{
    const Clip* clip = find_clip(ref);
    if (clip == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingClip, "clip not found")};
    }

    if (new_clip_id.empty() || clip_id_exists(clip_by_id_, new_clip_id)) {
        return {.error = make_error(TimelineErrorCode::DuplicateClip, "clip id already exists")};
    }

    if (split_time <= clip->range.start || split_time >= clip->range.end()) {
        return {.error = make_error(TimelineErrorCode::InvalidTimeRange, "split time must be inside clip range")};
    }

    const TimelineTick left_duration = split_time - clip->range.start;
    const TimelineTick right_duration = clip->range.end() - split_time;
    Clip right_clip{
        .id = std::move(new_clip_id),
        .range = TimeRange{.start = split_time, .duration = right_duration},
    };

    Clip* mutable_clip = find_clip_mut(ref);
    if (mutable_clip == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingClip, "clip not found")};
    }

    mutable_clip->range.duration = left_duration;
    Track* track = find_track_mut(ref.track_id);
    if (track == nullptr) {
        return {.error = make_error(TimelineErrorCode::MissingTrack, "track not found")};
    }

    Clip inserted_right = right_clip;
    track->clips.push_back(std::move(right_clip));
    sort_tracks();
    rebuild_index();
    return {.value = std::move(inserted_right)};
}

void TimelineIndex::sort_tracks()
{
    for (Track& track : tracks_) {
        std::sort(track.clips.begin(), track.clips.end(), [](const Clip& left, const Clip& right) {
            if (left.range.start != right.range.start) {
                return left.range.start < right.range.start;
            }
            return left.id < right.id;
        });
    }
}

void TimelineIndex::rebuild_index()
{
    track_by_id_.clear();
    clip_by_id_.clear();
    max_clip_end_by_track_.clear();
    max_clip_end_by_track_.reserve(tracks_.size());

    for (std::size_t track_index = 0; track_index < tracks_.size(); ++track_index) {
        const Track& track = tracks_[track_index];
        track_by_id_.emplace(track.id, track_index);
        std::vector<TimelineTick> max_end_for_track;
        max_end_for_track.reserve(track.clips.size());
        TimelineTick max_end = 0;

        for (std::size_t clip_index = 0; clip_index < track.clips.size(); ++clip_index) {
            clip_by_id_.emplace(track.clips[clip_index].id, std::pair{track_index, clip_index});
            max_end = std::max(max_end, track.clips[clip_index].range.end());
            max_end_for_track.push_back(max_end);
        }

        max_clip_end_by_track_.push_back(std::move(max_end_for_track));
    }
}

std::vector<SnapPoint> TimelineIndex::collect_snap_points(
    std::optional<TimelineTick> playhead,
    const std::vector<TimelineTick>& markers) const
{
    std::vector<SnapPoint> points;
    if (playhead) {
        points.push_back(SnapPoint{.time = *playhead, .type = SnapSourceType::Playhead, .label = "Playhead"});
    }
    for (const auto marker : markers) {
        points.push_back(SnapPoint{.time = marker, .type = SnapSourceType::Marker, .label = "Marker"});
    }
    for (const auto& track : tracks_) {
        for (const auto& clip : track.clips) {
            points.push_back(SnapPoint{.time = clip.range.start, .type = SnapSourceType::ClipStart, .label = clip.id});
            points.push_back(SnapPoint{.time = clip.range.end(), .type = SnapSourceType::ClipEnd, .label = clip.id});
        }
    }
    return points;
}

SnapResult TimelineIndex::snap_time(
    TimelineTick target_time,
    std::span<const SnapPoint> points,
    TimelineTick threshold_ticks) const
{
    SnapResult result{.snapped_time = target_time, .delta = 0, .snapped = false, .matched_point = std::nullopt};
    TimelineTick min_dist = threshold_ticks + 1;

    for (const auto& point : points) {
        TimelineTick dist = std::abs(point.time - target_time);
        if (dist <= threshold_ticks && dist < min_dist) {
            min_dist = dist;
            result.snapped_time = point.time;
            result.delta = point.time - target_time;
            result.snapped = true;
            result.matched_point = point;
        }
    }
    return result;
}

GroupMoveResult TimelineIndex::check_group_move(
    std::span<const GroupMoveItem> items) const
{
    GroupMoveResult result{.can_move = true, .conflicting_clip_ids = {}};

    for (std::size_t i = 0; i < items.size(); ++i) {
        TimeRange range_i{.start = items[i].new_start, .duration = items[i].duration};
        if (!range_i.is_valid()) {
            result.can_move = false;
            result.conflicting_clip_ids.push_back(items[i].clip_id);
            return result;
        }
        for (std::size_t j = i + 1; j < items.size(); ++j) {
            if (items[i].target_track_id == items[j].target_track_id) {
                TimeRange range_j{.start = items[j].new_start, .duration = items[j].duration};
                if (range_i.overlaps(range_j)) {
                    result.can_move = false;
                    result.conflicting_clip_ids.push_back(items[i].clip_id);
                    result.conflicting_clip_ids.push_back(items[j].clip_id);
                    return result;
                }
            }
        }
    }

    for (const auto& item : items) {
        const Track* track = find_track(item.target_track_id);
        if (track == nullptr) {
            result.can_move = false;
            result.conflicting_clip_ids.push_back(item.clip_id);
            continue;
        }

        TimeRange item_range{.start = item.new_start, .duration = item.duration};
        for (const auto& clip : track->clips) {
            bool is_moving_item = false;
            for (const auto& other_item : items) {
                if (other_item.clip_id == clip.id) {
                    is_moving_item = true;
                    break;
                }
            }
            if (is_moving_item) {
                continue;
            }
            if (item_range.overlaps(clip.range)) {
                result.can_move = false;
                result.conflicting_clip_ids.push_back(clip.id);
            }
        }
    }

    return result;
}

std::optional<TimelineTick> TimelineIndex::find_first_available_gap(
    std::string_view track_id,
    TimelineTick duration,
    TimelineTick min_start_time) const
{
    if (duration <= 0) {
        return min_start_time;
    }

    const auto* track = find_track(track_id);
    if (track == nullptr || track->clips.empty()) {
        return min_start_time;
    }

    TimelineTick cursor = min_start_time;

    for (const auto& clip : track->clips) {
        if (clip.range.end() <= cursor) {
            continue;
        }

        if (clip.range.start > cursor) {
            const TimelineTick gap = clip.range.start - cursor;
            if (gap >= duration) {
                return cursor;
            }
        }

        cursor = std::max(cursor, clip.range.end());
    }

    return cursor;
}

std::vector<std::pair<std::string, TimelineTick>> TimelineIndex::apply_ripple_shift(
    std::string_view track_id,
    TimelineTick after_time,
    TimelineTick delta_ticks)
{
    std::vector<std::pair<std::string, TimelineTick>> modified;
    auto* track = find_track_mut(track_id);
    if (track == nullptr || delta_ticks == 0) {
        return modified;
    }

    for (auto& clip : track->clips) {
        if (clip.range.start >= after_time) {
            const TimelineTick new_start = std::max(TimelineTick{0}, clip.range.start + delta_ticks);
            clip.range.start = new_start;
            modified.emplace_back(clip.id, new_start);
        }
    }

    if (!modified.empty()) {
        rebuild_index();
    }

    return modified;
}

} // namespace opencut

