#include "opencut/timeline.hpp"

#include "opencut/c_api.h"

#include <cassert>
#include <iostream>
#include <string>
#include <utility>

using opencut::Clip;
using opencut::ClipRef;
using opencut::TimeRange;
using opencut::TimelineErrorCode;
using opencut::TimelineIndex;
using opencut::Track;

namespace {

Clip clip(std::string id, opencut::TimelineTick start, opencut::TimelineTick duration)
{
    return Clip{.id = std::move(id), .range = TimeRange{.start = start, .duration = duration}};
}

void can_place_between_clips()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 10), clip("b", 20, 10)}},
    });

    assert(index.can_place("video-1", TimeRange{.start = 10, .duration = 10}));
    assert(!index.can_place("video-1", TimeRange{.start = 9, .duration = 2}));
    assert(!index.can_place("video-1", TimeRange{.start = 29, .duration = 2}));
}

void clips_in_visible_range()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 10), clip("b", 20, 10)}},
        Track{.id = "audio-1", .clips = {clip("c", 5, 10), clip("d", 40, 10)}},
    });

    const auto visible = index.clips_in_range(TimeRange{.start = 8, .duration = 17});
    assert(visible.size() == 3);
    assert(visible[0].clip_id == "a");
    assert(visible[1].clip_id == "c");
    assert(visible[2].clip_id == "b");
}

void move_clip_between_tracks()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 10)}},
        Track{.id = "video-2", .clips = {clip("b", 20, 10)}},
    });

    const auto moved = index.move_clip(
        ClipRef{.track_id = "video-1", .clip_id = "a"},
        "video-2",
        10);

    assert(moved.ok());
    assert(index.find_clip(ClipRef{.track_id = "video-1", .clip_id = "a"}) == nullptr);
    const Clip* target_clip = index.find_clip(ClipRef{.track_id = "video-2", .clip_id = "a"});
    assert(target_clip != nullptr);
    assert(target_clip->range.start == 10);
}

void reject_overlapping_move()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 10), clip("b", 20, 10)}},
    });

    const auto moved = index.move_clip(
        ClipRef{.track_id = "video-1", .clip_id = "a"},
        "video-1",
        25);

    assert(!moved.ok());
    assert(moved.error.code == TimelineErrorCode::Overlap);
}

void trim_clip()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 10), clip("b", 20, 10)}},
    });

    const auto trimmed = index.trim_clip(
        ClipRef{.track_id = "video-1", .clip_id = "a"},
        5,
        10);

    assert(trimmed.ok());
    const Clip* trimmed_clip = index.find_clip(ClipRef{.track_id = "video-1", .clip_id = "a"});
    assert(trimmed_clip != nullptr);
    assert(trimmed_clip->range.start == 5);
    assert(trimmed_clip->range.duration == 10);
}

void reject_invalid_time()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 10)}},
    });

    const auto moved = index.move_clip(
        ClipRef{.track_id = "video-1", .clip_id = "a"},
        "video-1",
        -1);

    assert(!moved.ok());
    assert(moved.error.code == TimelineErrorCode::InvalidTimeRange);
}

void insert_and_delete_clip()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {}},
    });

    const auto duplicate_track = index.add_track("video-1");
    assert(!duplicate_track.ok());
    assert(duplicate_track.error.code == TimelineErrorCode::DuplicateTrack);

    const auto inserted = index.insert_clip("video-1", clip("a", 0, 10));
    assert(inserted.ok());
    assert(index.find_clip(ClipRef{.track_id = "video-1", .clip_id = "a"}) != nullptr);

    const auto duplicate = index.insert_clip("video-1", clip("a", 20, 10));
    assert(!duplicate.ok());
    assert(duplicate.error.code == TimelineErrorCode::DuplicateClip);

    const auto deleted = index.delete_clip(ClipRef{.track_id = "video-1", .clip_id = "a"});
    assert(deleted.ok());
    assert(deleted.value.id == "a");
    assert(index.find_clip(ClipRef{.track_id = "video-1", .clip_id = "a"}) == nullptr);
}

void split_clip()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 100)}},
    });

    const auto split = index.split_clip(
        ClipRef{.track_id = "video-1", .clip_id = "a"},
        40,
        "b");

    assert(split.ok());
    assert(split.value.id == "b");
    assert(split.value.range.start == 40);
    assert(split.value.range.duration == 60);

    const Clip* left_clip = index.find_clip(ClipRef{.track_id = "video-1", .clip_id = "a"});
    const Clip* right_clip = index.find_clip(ClipRef{.track_id = "video-1", .clip_id = "b"});
    assert(left_clip != nullptr);
    assert(right_clip != nullptr);
    assert(left_clip->range.duration == 40);
    assert(right_clip->range.start == 40);
}

void c_api_smoke_test()
{
    OcTimeline* timeline = oc_timeline_create();
    assert(timeline != nullptr);

    assert(oc_timeline_add_track(timeline, "video-1") == OC_TIMELINE_OK);
    assert(oc_timeline_insert_clip(timeline, "video-1", "a", 0, 100) == OC_TIMELINE_OK);
    assert(oc_timeline_can_place(timeline, "video-1", 100, 10, nullptr) == 1);
    assert(oc_timeline_can_place(timeline, "video-1", 50, 10, nullptr) == 0);
    assert(oc_timeline_split_clip(timeline, "video-1", "a", 40, "b") == OC_TIMELINE_OK);
    assert(oc_timeline_count_clips_in_range(timeline, 0, 100) == 2);
    assert(oc_timeline_move_clip(timeline, "video-1", "b", "video-1", 120) == OC_TIMELINE_OK);
    assert(oc_timeline_trim_clip(timeline, "video-1", "a", 0, 20) == OC_TIMELINE_OK);
    assert(oc_timeline_delete_clip(timeline, "video-1", "a") == OC_TIMELINE_OK);

    oc_timeline_destroy(timeline);
}

void test_find_available_gap()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 50), clip("b", 100, 50)}},
    });

    // Gap of 30 starting from 0 -> fits in [50, 100] -> starts at 50
    const auto gap1 = index.find_first_available_gap("video-1", 30, 0);
    assert(gap1.has_value() && *gap1 == 50);

    // Gap of 60 starting from 0 -> [50, 100] is only 50 -> must go after 150
    const auto gap2 = index.find_first_available_gap("video-1", 60, 0);
    assert(gap2.has_value() && *gap2 == 150);
}

void test_apply_ripple_shift()
{
    TimelineIndex index({
        Track{.id = "video-1", .clips = {clip("a", 0, 50), clip("b", 100, 50)}},
    });

    const auto modified = index.apply_ripple_shift("video-1", 60, 30);
    assert(modified.size() == 1);
    assert(modified[0].first == "b");
    assert(modified[0].second == 130);

    // Check clip b is now at 130
    assert(index.can_place("video-1", TimeRange{.start = 100, .duration = 20}));
}

} // namespace

int main()
{
    can_place_between_clips();
    clips_in_visible_range();
    move_clip_between_tracks();
    reject_overlapping_move();
    trim_clip();
    reject_invalid_time();
    insert_and_delete_clip();
    split_clip();
    c_api_smoke_test();
    test_find_available_gap();
    test_apply_ripple_shift();

    std::cout << "opencut_core timeline tests passed\n";
    return 0;
}
