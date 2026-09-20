#include "core/time/TimelineTime.h"
#include "core/time/Timecode.h"
#include <cassert>
#include <iostream>

using namespace catchim::core;

void testTimelineTime() {
    TimelineTime t1 = TimelineTime::fromSeconds(1.0);
    assert(t1.ticks() == 120'000);
    assert(t1.toSeconds() == 1.0);

    TimelineTime t2 = TimelineTime::fromSeconds(0.5);
    assert(t2.ticks() == 60'000);

    TimelineTime t3 = t1 + t2;
    assert(t3.ticks() == 180'000);
    assert(t3.toSeconds() == 1.5);

    // Frame rounding at 30 fps (4000 ticks/frame)
    FrameRate fps30{30, 1};
    TimelineTime unaligned = TimelineTime::fromTicks(4100);
    TimelineTime rounded = unaligned.roundToFrame(fps30);
    assert(rounded.ticks() == 4000);

    TimelineTime unaligned2 = TimelineTime::fromTicks(5999);
    TimelineTime rounded2 = unaligned2.roundToFrame(fps30);
    assert(rounded2.ticks() == 4000);

    TimelineTime unaligned3 = TimelineTime::fromTicks(6000);
    TimelineTime rounded3 = unaligned3.roundToFrame(fps30);
    assert(rounded3.ticks() == 8000);

    std::cout << "[PASS] testTimelineTime" << std::endl;
}

void testTimecode() {
    FrameRate fps30{30, 1};
    TimelineTime t = TimelineTime::fromSeconds(125.5); // 00:02:05:15 at 30fps

    std::string tc = Timecode::format(t, TimecodeFormat::HH_MM_SS_FF, fps30);
    assert(tc == "00:02:05:15");

    auto parsedOpt = Timecode::parse(tc, TimecodeFormat::HH_MM_SS_FF, fps30);
    assert(parsedOpt.has_value());
    assert(parsedOpt->toSeconds() == 125.5);

    // Test MM:SS
    std::string tcMmSs = Timecode::format(t, TimecodeFormat::MM_SS, fps30);
    assert(tcMmSs == "02:05");

    std::cout << "[PASS] testTimecode" << std::endl;
}

int main() {
    testTimelineTime();
    testTimecode();
    std::cout << "All TimeTests passed successfully!" << std::endl;
    return 0;
}
