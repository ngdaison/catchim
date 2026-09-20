#pragma once

#include "render/scene/FrameDescriptorBuilder.h"
#include <vector>
#include <cstdint>
#include <string>
#include <array>
#include <memory>

namespace catchim::render {

/**
 * @brief Represents an offscreen 32-bit RGBA pixel surface buffer.
 * Corresponds to web/src/services/renderer/canvas-utils.ts and offscreen canvas management.
 */
class RenderSurface {
public:
    RenderSurface() = default;
    RenderSurface(int width, int height);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }
    int stride() const noexcept { return width_ * 4; }
    size_t byteSize() const noexcept { return pixels_.size() * sizeof(uint32_t); }

    const uint32_t* data() const noexcept { return pixels_.data(); }
    uint32_t* data() noexcept { return pixels_.data(); }

    void resize(int width, int height);
    void clear(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0) noexcept;
    void clear(const std::array<double, 4>& color) noexcept;
    void fill(uint32_t rgba) noexcept;

    void setPixel(int x, int y, uint32_t rgba) noexcept;
    uint32_t getPixel(int x, int y) const noexcept;

    void copyFrom(const RenderSurface& src, int dstX = 0, int dstY = 0) noexcept;

    void blendOver(
        const RenderSurface& src,
        const QuadTransformDescriptor& transform,
        double opacity = 1.0,
        const std::string& blendMode = "normal"
    ) noexcept;

    static uint32_t makeRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept {
        return static_cast<uint32_t>(r) |
               (static_cast<uint32_t>(g) << 8) |
               (static_cast<uint32_t>(b) << 16) |
               (static_cast<uint32_t>(a) << 24);
    }

    static void unpackRgba(uint32_t rgba, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) noexcept {
        r = static_cast<uint8_t>(rgba & 0xFF);
        g = static_cast<uint8_t>((rgba >> 8) & 0xFF);
        b = static_cast<uint8_t>((rgba >> 16) & 0xFF);
        a = static_cast<uint8_t>((rgba >> 24) & 0xFF);
    }

private:
    int width_{0};
    int height_{0};
    std::vector<uint32_t> pixels_;
};

} // namespace catchim::render
