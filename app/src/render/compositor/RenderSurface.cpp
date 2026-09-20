#include "render/compositor/RenderSurface.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::render {

RenderSurface::RenderSurface(int width, int height) {
    resize(width, height);
}

void RenderSurface::resize(int width, int height) {
    if (width < 0) width = 0;
    if (height < 0) height = 0;

    width_ = width;
    height_ = height;
    pixels_.assign(static_cast<size_t>(width_) * static_cast<size_t>(height_), 0);
}

void RenderSurface::clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    fill(makeRgba(r, g, b, a));
}

void RenderSurface::clear(const std::array<double, 4>& color) noexcept {
    auto clampCh = [](double c) -> uint8_t {
        return static_cast<uint8_t>(std::clamp(std::round(c * 255.0), 0.0, 255.0));
    };
    uint8_t r = clampCh(color[0]);
    uint8_t g = clampCh(color[1]);
    uint8_t b = clampCh(color[2]);
    uint8_t a = clampCh(color[3]);
    clear(r, g, b, a);
}

void RenderSurface::fill(uint32_t rgba) noexcept {
    std::fill(pixels_.begin(), pixels_.end(), rgba);
}

void RenderSurface::setPixel(int x, int y, uint32_t rgba) noexcept {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[static_cast<size_t>(y) * width_ + x] = rgba;
    }
}

uint32_t RenderSurface::getPixel(int x, int y) const noexcept {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        return pixels_[static_cast<size_t>(y) * width_ + x];
    }
    return 0;
}

void RenderSurface::copyFrom(const RenderSurface& src, int dstX, int dstY) noexcept {
    if (src.width_ <= 0 || src.height_ <= 0 || width_ <= 0 || height_ <= 0) {
        return;
    }

    int startX = std::max(0, dstX);
    int startY = std::max(0, dstY);
    int endX = std::min(width_, dstX + src.width_);
    int endY = std::min(height_, dstY + src.height_);

    for (int y = startY; y < endY; ++y) {
        int srcY = y - dstY;
        for (int x = startX; x < endX; ++x) {
            int srcX = x - dstX;
            setPixel(x, y, src.getPixel(srcX, srcY));
        }
    }
}

void RenderSurface::blendOver(
    const RenderSurface& src,
    const QuadTransformDescriptor& transform,
    double opacity,
    const std::string& blendMode
) noexcept {
    if (width_ <= 0 || height_ <= 0 || src.width() <= 0 || src.height() <= 0) {
        return;
    }
    if (transform.width <= 0.0 || transform.height <= 0.0 || opacity <= 0.0) {
        return;
    }

    double rad = -transform.rotationDegrees * M_PI / 180.0;
    double cosR = std::cos(rad);
    double sinR = std::sin(rad);

    double halfW = transform.width / 2.0;
    double halfH = transform.height / 2.0;
    double radius = std::sqrt(halfW * halfW + halfH * halfH);

    int minX = std::max(0, static_cast<int>(std::floor(transform.centerX - radius)));
    int maxX = std::min(width_ - 1, static_cast<int>(std::ceil(transform.centerX + radius)));
    int minY = std::max(0, static_cast<int>(std::floor(transform.centerY - radius)));
    int maxY = std::min(height_ - 1, static_cast<int>(std::ceil(transform.centerY + radius)));

    double invW = 1.0 / transform.width;
    double invH = 1.0 / transform.height;
    double clampedOpacity = std::clamp(opacity, 0.0, 1.0);

    for (int dy = minY; dy <= maxY; ++dy) {
        double py = dy - transform.centerY;
        for (int dx = minX; dx <= maxX; ++dx) {
            double px = dx - transform.centerX;

            // Unrotate
            double rx = px * cosR - py * sinR;
            double ry = px * sinR + py * cosR;

            double u = rx * invW;
            double v = ry * invH;

            if (transform.flipX) u = -u;
            if (transform.flipY) v = -v;

            if (u < -0.5 || u > 0.5 || v < -0.5 || v > 0.5) {
                continue;
            }

            double sx = (u + 0.5) * (src.width() - 1);
            double sy = (v + 0.5) * (src.height() - 1);

            int isx = std::clamp(static_cast<int>(std::round(sx)), 0, src.width() - 1);
            int isy = std::clamp(static_cast<int>(std::round(sy)), 0, src.height() - 1);

            uint32_t srcPixel = src.getPixel(isx, isy);
            uint8_t sr, sg, sb, sa;
            unpackRgba(srcPixel, sr, sg, sb, sa);

            if (sa == 0) {
                continue;
            }

            double effSrcA = (sa / 255.0) * clampedOpacity;
            if (effSrcA <= 0.0) {
                continue;
            }

            uint32_t dstPixel = getPixel(dx, dy);
            uint8_t dr, dg, db, da;
            unpackRgba(dstPixel, dr, dg, db, da);
            double dstA = da / 255.0;

            double blendedR = sr;
            double blendedG = sg;
            double blendedB = sb;

            if (blendMode == "multiply") {
                blendedR = (sr * dr) / 255.0;
                blendedG = (sg * dg) / 255.0;
                blendedB = (sb * db) / 255.0;
            } else if (blendMode == "screen") {
                blendedR = 255.0 - ((255.0 - sr) * (255.0 - dr)) / 255.0;
                blendedG = 255.0 - ((255.0 - sg) * (255.0 - dg)) / 255.0;
                blendedB = 255.0 - ((255.0 - sb) * (255.0 - db)) / 255.0;
            } else if (blendMode == "overlay") {
                auto overlayCh = [](double s, double d) {
                    return (d < 128.0) ? (2.0 * s * d) / 255.0
                                       : 255.0 - (2.0 * (255.0 - s) * (255.0 - d)) / 255.0;
                };
                blendedR = overlayCh(sr, dr);
                blendedG = overlayCh(sg, dg);
                blendedB = overlayCh(sb, db);
            }

            // Standard alpha compositing: out = src * srcA + dst * dstA * (1 - srcA)
            double outA = effSrcA + dstA * (1.0 - effSrcA);
            if (outA > 0.0) {
                double outR = (blendedR * effSrcA + dr * dstA * (1.0 - effSrcA)) / outA;
                double outG = (blendedG * effSrcA + dg * dstA * (1.0 - effSrcA)) / outA;
                double outB = (blendedB * effSrcA + db * dstA * (1.0 - effSrcA)) / outA;

                uint8_t finalR = static_cast<uint8_t>(std::clamp(std::round(outR), 0.0, 255.0));
                uint8_t finalG = static_cast<uint8_t>(std::clamp(std::round(outG), 0.0, 255.0));
                uint8_t finalB = static_cast<uint8_t>(std::clamp(std::round(outB), 0.0, 255.0));
                uint8_t finalA = static_cast<uint8_t>(std::clamp(std::round(outA * 255.0), 0.0, 255.0));

                setPixel(dx, dy, makeRgba(finalR, finalG, finalB, finalA));
            }
        }
    }
}

} // namespace catchim::render
