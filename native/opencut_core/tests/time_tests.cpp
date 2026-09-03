#include "opencut/time.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut;

void test_frame_rate() {
    auto fps30 = FrameRate::fps_30();
    assert(fps30.is_valid());
    assert(fps30.as_f64().value() == 30.0);
    assert(fps30.ticks_per_frame().value() == 4000); // 120,000 / 30 = 4,000
    assert(fps30.frame_number_upper_bound().value() == 30);

    auto fps60 = FrameRate::fps_60();
    assert(fps60.ticks_per_frame().value() == 2000);

    auto fps24 = FrameRate::fps_24();
    assert(fps24.ticks_per_frame().value() == 5000);

    auto fps25 = FrameRate::fps_25();
    assert(fps25.ticks_per_frame().value() == 4800);

    auto fps_invalid = FrameRate{0, 1};
    assert(!fps_invalid.is_valid());
    assert(!fps_invalid.as_f64().has_value());
}

void test_media_time_basics() {
    auto t0 = MediaTime::zero();
    assert(t0.as_ticks() == 0);
    assert(t0.to_seconds_f64() == 0.0);

    auto t1 = MediaTime::from_seconds_f64(1.5);
    assert(t1.has_value());
    assert(t1->as_ticks() == 180000);
    assert(std::abs(t1->to_seconds_f64() - 1.5) < 1e-9);

    auto t_add = *t1 + MediaTime::from_ticks(20000);
    assert(t_add.as_ticks() == 200000);

    auto t_sub = t_add - *t1;
    assert(t_sub.as_ticks() == 20000);

    auto t_clamped = MediaTime::from_ticks(50000).clamp(MediaTime::from_ticks(0), MediaTime::from_ticks(40000));
    assert(t_clamped.as_ticks() == 40000);
}

void test_frame_alignment_and_rounding() {
    auto fps = FrameRate::fps_30(); // 4000 ticks per frame

    auto t_frame0 = MediaTime::from_frame(0, fps);
    assert(t_frame0->as_ticks() == 0);

    auto t_frame5 = MediaTime::from_frame(5, fps);
    assert(t_frame5->as_ticks() == 20000);
    assert(t_frame5->to_frame_floor(fps).value() == 5);
    assert(t_frame5->to_frame_round(fps).value() == 5);
    assert(t_frame5->is_frame_aligned(fps).value() == true);

    // 2100 ticks: 20000 + 1000 ticks -> floor is 5, round is 5
    auto t_unaligned1 = MediaTime::from_ticks(21000);
    assert(t_unaligned1.to_frame_floor(fps).value() == 5);
    assert(t_unaligned1.to_frame_round(fps).value() == 5);
    assert(t_unaligned1.is_frame_aligned(fps).value() == false);
    assert(t_unaligned1.floor_to_frame(fps)->as_ticks() == 20000);
    assert(t_unaligned1.round_to_frame(fps)->as_ticks() == 20000);

    // 22000 ticks: halfway (remainder 2000 >= 2000) -> round is 6
    auto t_unaligned2 = MediaTime::from_ticks(22000);
    assert(t_unaligned2.to_frame_round(fps).value() == 6);
    assert(t_unaligned2.round_to_frame(fps)->as_ticks() == 24000);

    // last_frame_time
    auto duration = MediaTime::from_ticks(40000); // exactly 10 frames
    auto last_frame = duration.last_frame_time(fps);
    assert(last_frame.has_value());
    assert(last_frame->as_ticks() == 36000); // 9th frame (0-indexed)
}

void test_timecode_formatting_and_parsing() {
    auto fps = FrameRate::fps_30();

    // 1 hour, 2 minutes, 3 seconds, 15 frames = 3600 + 120 + 3 = 3723s + 15 frames
    // 3723 * 120000 + 15 * 4000 = 446760000 + 60000 = 446820000 ticks
    auto t = MediaTime::from_ticks(446820000);

    auto tc_ff = format_timecode(t, TimeCodeFormat::HhMmSsFf, fps);
    assert(tc_ff.has_value());
    assert(*tc_ff == "01:02:03:15");

    auto parsed_ff = parse_timecode("01:02:03:15", TimeCodeFormat::HhMmSsFf, fps);
    assert(parsed_ff.has_value());
    assert(parsed_ff->as_ticks() == t.as_ticks());

    // MM:SS
    auto t_short = MediaTime::from_ticks(125 * TICKS_PER_SECOND); // 2m 5s
    auto tc_mmss = format_timecode(t_short, TimeCodeFormat::MmSs);
    assert(tc_mmss.has_value());
    assert(*tc_mmss == "02:05");

    auto parsed_mmss = parse_timecode("02:05", TimeCodeFormat::MmSs);
    assert(parsed_mmss.has_value());
    assert(parsed_mmss->as_ticks() == t_short.as_ticks());

    // Guess format
    assert(guess_timecode_format("02:05") == TimeCodeFormat::MmSs);
    assert(guess_timecode_format("01:02:05") == TimeCodeFormat::HhMmSs);
    assert(guess_timecode_format("01:02:03:15") == TimeCodeFormat::HhMmSsFf);
}

int main() {
    test_frame_rate();
    test_media_time_basics();
    test_frame_alignment_and_rounding();
    test_timecode_formatting_and_parsing();

    std::cout << "All time tests passed successfully!\n";
    return 0;
}
