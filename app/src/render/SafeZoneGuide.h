#pragma once

#include "render/ShapeRenderer.h"
#include <cstdint>

namespace catchim::render {

enum class GuideType {
    RuleOfThirds,
    Crosshair,
    TikTok,
    InstagramReels,
    YouTubeShorts
};

struct SafeZoneMargin {
    double topRatio{0.0};
    double bottomRatio{0.0};
    double leftRatio{0.0};
    double rightRatio{0.0};
};

class SafeZoneGuide {
public:
    static SafeZoneMargin getMargins(GuideType type) noexcept;

    static void renderGuideOverlay(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        GuideType type,
        ColorRGBA guideColor = ColorRGBA::fromRgb(255, 255, 255, 180)
    ) noexcept;

private:
    static void drawLine(
        uint8_t* buffer,
        int32_t w,
        int32_t h,
        int32_t x0,
        int32_t y0,
        int32_t x1,
        int32_t y1,
        ColorRGBA color
    ) noexcept;
};

} // namespace catchim::render
