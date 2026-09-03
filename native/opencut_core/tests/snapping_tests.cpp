#include "opencut/timeline.hpp"

#include <cassert>
#include <iostream>

using namespace opencut;

void test_snapping() {
    TimelineIndex index({
        Track{.id = "v1", .clips = {
            Clip{.id = "c1", .range = TimeRange{.start = 0, .duration = 1000}},
            Clip{.id = "c2", .range = TimeRange{.start = 2000, .duration = 1000}},
        }}
    });

    auto points = index.collect_snap_points(500, {1500});
    // Expected snap points:
    // Playhead: 500
    // Marker: 1500
    // c1 start: 0, c1 end: 1000
    // c2 start: 2000, c2 end: 3000
    assert(points.size() == 6);

    // Target time 1010 within threshold 20 -> snaps to 1000 (delta = -10)
    auto snap1 = index.snap_time(1010, points, 20);
    assert(snap1.snapped == true);
    assert(snap1.snapped_time == 1000);
    assert(snap1.delta == -10);

    // Target time 1050 with threshold 20 -> does not snap
    auto snap2 = index.snap_time(1050, points, 20);
    assert(snap2.snapped == false);
    assert(snap2.snapped_time == 1050);
}

void test_group_move() {
    TimelineIndex index({
        Track{.id = "v1", .clips = {
            Clip{.id = "c1", .range = TimeRange{.start = 0, .duration = 1000}},
            Clip{.id = "c2", .range = TimeRange{.start = 2000, .duration = 1000}},
        }},
        Track{.id = "v2", .clips = {
            Clip{.id = "c3", .range = TimeRange{.start = 500, .duration = 1000}},
        }}
    });

    // Move c1 and c2 forward by 500 on v1 -> new c1: 500..1500, new c2: 2500..3500
    // They don't overlap each other and they are moving together
    std::vector<GroupMoveItem> items = {
        GroupMoveItem{.clip_id = "c1", .source_track_id = "v1", .target_track_id = "v1", .new_start = 500, .duration = 1000},
        GroupMoveItem{.clip_id = "c2", .source_track_id = "v1", .target_track_id = "v1", .new_start = 2500, .duration = 1000},
    };
    auto res = index.check_group_move(items);
    assert(res.can_move == true);

    // Move c1 to v2 at 800..1800 -> overlaps non-moving c3 (500..1500)
    std::vector<GroupMoveItem> items_conflict = {
        GroupMoveItem{.clip_id = "c1", .source_track_id = "v1", .target_track_id = "v2", .new_start = 800, .duration = 1000},
    };
    auto res_conflict = index.check_group_move(items_conflict);
    assert(res_conflict.can_move == false);
    assert(!res_conflict.conflicting_clip_ids.empty());
}

int main() {
    test_snapping();
    test_group_move();
    std::cout << "All snapping and group move tests passed successfully!\n";
    return 0;
}
