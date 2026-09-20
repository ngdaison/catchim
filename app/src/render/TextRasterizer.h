#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace catchim::render {

struct TextProperties {
    std::string text{"Catchim Text"};
    int32_t fontSize{32};
    uint8_t textR{255};
    uint8_t textG{255};
    uint8_t textB{255};
    uint8_t textA{255};

    uint8_t bgR{0};
    uint8_t bgG{0};
    uint8_t bgB{0};
    uint8_t bgA{0}; // 0 = transparent

    int32_t padding{12};
    std::string alignment{"center"}; // "left", "center", "right"
};

class TextRasterizer {
public:
    static std::vector<uint8_t> rasterize(
        const TextProperties& props,
        int32_t& outWidth,
        int32_t& outHeight
    );
};

} // namespace catchim::render
