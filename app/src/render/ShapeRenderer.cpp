#include "render/ShapeRenderer.h"

namespace catchim::render {

void ShapeRenderer::blendPixel(uint8_t* dst, ColorRGBA src) noexcept {
    if (src.a == 0) return;

    if (src.a == 255) {
        dst[0] = src.r;
        dst[1] = src.g;
        dst[2] = src.b;
        dst[3] = 255;
        return;
    }

    float a = src.a / 255.0f;
    float invA = 1.0f - a;

    dst[0] = static_cast<uint8_t>(src.r * a + dst[0] * invA);
    dst[1] = static_cast<uint8_t>(src.g * a + dst[1] * invA);
    dst[2] = static_cast<uint8_t>(src.b * a + dst[2] * invA);
    dst[3] = static_cast<uint8_t>(std::clamp(dst[3] + src.a * (255 - dst[3]) / 255, 0, 255));
}

void ShapeRenderer::drawSolid(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    ColorRGBA color
) noexcept {
    if (!buffer || w <= 0 || h <= 0) return;
    size_t total = static_cast<size_t>(w) * h;
    for (size_t i = 0; i < total; ++i) {
        buffer[i * 4 + 0] = color.r;
        buffer[i * 4 + 1] = color.g;
        buffer[i * 4 + 2] = color.b;
        buffer[i * 4 + 3] = color.a;
    }
}

void ShapeRenderer::drawLinearGradient(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    ColorRGBA startColor,
    ColorRGBA endColor,
    double angleDeg
) noexcept {
    if (!buffer || w <= 0 || h <= 0) return;

    double rad = angleDeg * 3.14159265358979323846 / 180.0;
    double dx = std::cos(rad);
    double dy = std::sin(rad);

    double halfW = w / 2.0;
    double halfH = h / 2.0;
    double L = 0.5 * (std::abs(w * dx) + std::abs(h * dy));
    if (L < 0.001) L = 1.0;

    for (int32_t y = 0; y < h; ++y) {
        double py = y - halfH;
        for (int32_t x = 0; x < w; ++x) {
            double px = x - halfW;
            double proj = px * dx + py * dy;
            double t = std::clamp((proj / L + 1.0) * 0.5, 0.0, 1.0);

            ColorRGBA c;
            c.r = static_cast<uint8_t>(startColor.r + (endColor.r - startColor.r) * t);
            c.g = static_cast<uint8_t>(startColor.g + (endColor.g - startColor.g) * t);
            c.b = static_cast<uint8_t>(startColor.b + (endColor.b - startColor.b) * t);
            c.a = static_cast<uint8_t>(startColor.a + (endColor.a - startColor.a) * t);

            size_t idx = (static_cast<size_t>(y) * w + x) * 4;
            blendPixel(&buffer[idx], c);
        }
    }
}

void ShapeRenderer::drawRadialGradient(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    ColorRGBA centerColor,
    ColorRGBA edgeColor,
    int32_t radius
) noexcept {
    if (!buffer || w <= 0 || h <= 0) return;

    double centerX = w / 2.0;
    double centerY = h / 2.0;
    double maxRadius = radius > 0 ? static_cast<double>(radius) : std::hypot(centerX, centerY);
    if (maxRadius < 0.001) maxRadius = 1.0;

    for (int32_t y = 0; y < h; ++y) {
        double py = y - centerY;
        for (int32_t x = 0; x < w; ++x) {
            double px = x - centerX;
            double dist = std::hypot(px, py);
            double t = std::clamp(dist / maxRadius, 0.0, 1.0);

            ColorRGBA c;
            c.r = static_cast<uint8_t>(centerColor.r + (edgeColor.r - centerColor.r) * t);
            c.g = static_cast<uint8_t>(centerColor.g + (edgeColor.g - centerColor.g) * t);
            c.b = static_cast<uint8_t>(centerColor.b + (edgeColor.b - centerColor.b) * t);
            c.a = static_cast<uint8_t>(centerColor.a + (edgeColor.a - centerColor.a) * t);

            size_t idx = (static_cast<size_t>(y) * w + x) * 4;
            blendPixel(&buffer[idx], c);
        }
    }
}

void ShapeRenderer::drawRectangle(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    int32_t rx,
    int32_t ry,
    int32_t rw,
    int32_t rh,
    ColorRGBA fill,
    ColorRGBA stroke,
    int32_t strokeWidth,
    int32_t cornerRadius
) noexcept {
    if (!buffer || w <= 0 || h <= 0 || rw <= 0 || rh <= 0) return;

    int32_t minX = std::max(0, rx);
    int32_t maxX = std::min(w - 1, rx + rw - 1);
    int32_t minY = std::max(0, ry);
    int32_t maxY = std::min(h - 1, ry + rh - 1);

    int32_t cr = std::clamp(cornerRadius, 0, std::min(rw, rh) / 2);
    int32_t sw = std::max(0, strokeWidth);

    double halfW = rw / 2.0;
    double halfH = rh / 2.0;
    double centerX = rx + halfW;
    double centerY = ry + halfH;

    for (int32_t y = minY; y <= maxY; ++y) {
        for (int32_t x = minX; x <= maxX; ++x) {
            bool inside = true;

            // Rounded corner check using 2D signed distance
            if (cr > 0) {
                double qx = std::abs(x - centerX) - (halfW - cr);
                double qy = std::abs(y - centerY) - (halfH - cr);
                if (qx > 0 && qy > 0) {
                    double dist = std::hypot(qx, qy);
                    if (dist > cr) {
                        inside = false;
                    }
                }
            }

            if (!inside) continue;

            // Determine if on stroke or in fill
            bool isStroke = false;
            if (sw > 0 && stroke.a > 0) {
                int32_t distLeft = x - rx;
                int32_t distRight = (rx + rw - 1) - x;
                int32_t distTop = y - ry;
                int32_t distBottom = (ry + rh - 1) - y;

                int32_t minDistToEdge = std::min({distLeft, distRight, distTop, distBottom});
                if (minDistToEdge < sw) {
                    isStroke = true;
                }
            }

            size_t idx = (static_cast<size_t>(y) * w + x) * 4;
            if (isStroke) {
                blendPixel(&buffer[idx], stroke);
            } else if (fill.a > 0) {
                blendPixel(&buffer[idx], fill);
            }
        }
    }
}

void ShapeRenderer::drawEllipse(
    uint8_t* buffer,
    int32_t w,
    int32_t h,
    int32_t cx,
    int32_t cy,
    int32_t radiusX,
    int32_t radiusY,
    ColorRGBA fill,
    ColorRGBA stroke,
    int32_t strokeWidth
) noexcept {
    if (!buffer || w <= 0 || h <= 0 || radiusX <= 0 || radiusY <= 0) return;

    int32_t minX = std::max(0, cx - radiusX);
    int32_t maxX = std::min(w - 1, cx + radiusX);
    int32_t minY = std::max(0, cy - radiusY);
    int32_t maxY = std::min(h - 1, cy + radiusY);

    int32_t sw = std::max(0, strokeWidth);
    int32_t innerRx = std::max(0, radiusX - sw);
    int32_t innerRy = std::max(0, radiusY - sw);

    for (int32_t y = minY; y <= maxY; ++y) {
        double dy = y - cy;
        for (int32_t x = minX; x <= maxX; ++x) {
            double dx = x - cx;

            double normDistOuter = (dx * dx) / (static_cast<double>(radiusX) * radiusX)
                                 + (dy * dy) / (static_cast<double>(radiusY) * radiusY);

            if (normDistOuter <= 1.0) {
                bool isStroke = false;
                if (sw > 0 && stroke.a > 0) {
                    if (innerRx == 0 || innerRy == 0) {
                        isStroke = true;
                    } else {
                        double normDistInner = (dx * dx) / (static_cast<double>(innerRx) * innerRx)
                                             + (dy * dy) / (static_cast<double>(innerRy) * innerRy);
                        if (normDistInner > 1.0) {
                            isStroke = true;
                        }
                    }
                }

                size_t idx = (static_cast<size_t>(y) * w + x) * 4;
                if (isStroke) {
                    blendPixel(&buffer[idx], stroke);
                } else if (fill.a > 0) {
                    blendPixel(&buffer[idx], fill);
                }
            }
        }
    }
}

} // namespace catchim::render
