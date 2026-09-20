#pragma once

#include "core/time/TimelineTime.h"
#include <string>

namespace catchim::editor {

enum class KeyframeInterpolation {
    Linear,
    Bezier,
    Hold
};

struct KeyframeHandle {
    double x{0.0};
    double y{0.0};
};

struct Keyframe {
    core::TimelineTime time{0};
    double value{0.0};
    KeyframeInterpolation interpolation{KeyframeInterpolation::Linear};
    KeyframeHandle leftHandle{-0.2, 0.0};
    KeyframeHandle rightHandle{0.2, 0.0};

    // Normalized cubic bezier control points for segment between this keyframe and the next
    double bezierX1{0.25};
    double bezierY1{0.1};
    double bezierX2{0.25};
    double bezierY2{1.0};

    bool operator<(const Keyframe& other) const {
        return time < other.time;
    }
};

} // namespace catchim::editor
