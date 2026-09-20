#pragma once

#include <cstdint>
#include <algorithm>

namespace catchim::render {

enum class TransitionType {
    Crossfade,
    FadeToBlack,
    FadeToWhite,
    SlideLeft,
    SlideRight,
    WipeLeft,
    WipeRight,
    ZoomIn
};

class TransitionEngine {
public:
    static void blend(
        uint8_t* dst,
        const uint8_t* frameA,
        const uint8_t* frameB,
        int32_t w,
        int32_t h,
        TransitionType type,
        double progress
    ) noexcept;
};

} // namespace catchim::render
