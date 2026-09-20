#include "render/SafeZoneGuide.h"
#include <cmath>
#include <algorithm>

namespace catchim::render {

SafeZoneMargin SafeZoneGuide::getMargins(GuideType type) noexcept {
    switch (type) {
        case GuideType::TikTok:
            return {0.12, 0.22, 0.05, 0.18};
        case GuideType::InstagramReels:
            return {0.10, 0.20, 0.05, 0.15};
        case GuideType::YouTubeShorts:
            return {0.08, 0.18, 0.05, 0.15};
        case GuideType::RuleOfThirds:
        case GuideType::Crosshair:
        default:
            return {0.0, 0.0, 0.0, 0.0};
    }
}

void SafeZoneGuide::drawLine(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    int32_t x0,
    int32_t y0,
    int32_t x1,
    int32_t y1,
    ColorRGBA color
) noexcept {
    if (!buffer || w <= 0 || h <= 0) return;

    int32_t dx = std::abs(x1 - x0);
    int32_t dy = std::abs(y1 - y0);
    int32_t sx = (x0 < x1) ? 1 : -1;
    int32_t sy = (y0 < y1) ? 1 : -1;
    int32_t err = dx - dy;

    float a = color.a / 255.0f;
    float invA = 1.0f - a;

    while (true) {
        if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
            size_t idx = (static_cast<size_t>(y0) * w + x0) * 4;
            buffer[idx + 0] = static_cast<uint8_t>(color.r * a + buffer[idx + 0] * invA);
            buffer[idx + 1] = static_cast<uint8_t>(color.g * a + buffer[idx + 1] * invA);
            buffer[idx + 2] = static_cast<uint8_t>(color.b * a + buffer[idx + 2] * invA);
            buffer[idx + 3] = 255;
        }

        if (x0 == x1 && y0 == y1) break;
        int32_t e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void SafeZoneGuide::renderGuideOverlay(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    GuideType type,
    ColorRGBA guideColor
) noexcept {
    if (!buffer || w <= 0 || h <= 0) return;

    if (type == GuideType::RuleOfThirds) {
        int32_t x1 = w / 3;
        int32_t x2 = (2 * w) / 3;
        int32_t y1 = h / 3;
        int32_t y2 = (2 * h) / 3;

        drawLine(buffer, w, h, x1, 0, x1, h - 1, guideColor);
        drawLine(buffer, w, h, x2, 0, x2, h - 1, guideColor);
        drawLine(buffer, w, h, 0, y1, w - 1, y1, guideColor);
        drawLine(buffer, w, h, 0, y2, w - 1, y2, guideColor);
        return;
    }

    if (type == GuideType::Crosshair) {
        int32_t midX = w / 2;
        int32_t midY = h / 2;
        drawLine(buffer, w, h, midX, 0, midX, h - 1, guideColor);
        drawLine(buffer, w, h, 0, midY, w - 1, midY, guideColor);
        return;
    }

    // Platform safe zones
    SafeZoneMargin m = getMargins(type);
    int32_t leftX = static_cast<int32_t>(w * m.leftRatio);
    int32_t rightX = static_cast<int32_t>(w * (1.0 - m.rightRatio));
    int32_t topY = static_cast<int32_t>(h * m.topRatio);
    int32_t bottomY = static_cast<int32_t>(h * (1.0 - m.bottomRatio));

    // Draw safe rectangle boundary
    drawLine(buffer, w, h, leftX, topY, rightX, topY, guideColor);
    drawLine(buffer, w, h, leftX, bottomY, rightX, bottomY, guideColor);
    drawLine(buffer, w, h, leftX, topY, leftX, bottomY, guideColor);
    drawLine(buffer, w, h, rightX, topY, rightX, bottomY, guideColor);

    // Darken unsafe margins lightly (40 alpha)
    ColorRGBA dimColor = ColorRGBA::fromRgb(0, 0, 0, 40);
    float da = dimColor.a / 255.0f;
    float invDa = 1.0f - da;

    for (int32_t y = 0; y < h; ++y) {
        bool outsideY = (y < topY || y > bottomY);
        for (int32_t x = 0; x < w; ++x) {
            bool outsideX = (x < leftX || x > rightX);
            if (outsideY || outsideX) {
                size_t idx = (static_cast<size_t>(y) * w + x) * 4;
                buffer[idx + 0] = static_cast<uint8_t>(buffer[idx + 0] * invDa);
                buffer[idx + 1] = static_cast<uint8_t>(buffer[idx + 1] * invDa);
                buffer[idx + 2] = static_cast<uint8_t>(buffer[idx + 2] * invDa);
            }
        }
    }
}

} // namespace catchim::render
