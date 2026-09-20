#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace catchim::render {

enum class MaskShape {
    Rectangle,
    Ellipse,
    Star,
    Heart
};

struct MaskDefinition {
    MaskShape shape{MaskShape::Rectangle};
    double centerX{0.5};    // Normalized 0.0 - 1.0
    double centerY{0.5};
    double width{1.0};      // Normalized 0.0 - 1.0
    double height{1.0};
    double cornerRadius{0.0}; // Normalized 0.0 - 0.5
    double feather{0.0};     // Soft edge in pixels
    bool inverted{false};
};

class MaskEngine {
public:
    // Generate an 8-bit grayscale alpha mask buffer (0 = fully transparent, 255 = fully opaque)
    static std::vector<uint8_t> generateMask(
        const MaskDefinition& mask,
        int32_t width,
        int32_t height
    );

    // Apply alpha mask directly to an RGBA pixel buffer
    static void applyMaskToLayer(
        uint8_t* rgbaPixels,
        const uint8_t* alphaMask,
        int32_t width,
        int32_t height
    );
};

} // namespace catchim::render
