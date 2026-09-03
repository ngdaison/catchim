#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace opencut::text {

enum class TextAlign : uint32_t {
    Left = 0,
    Center = 1,
    Right = 2,
    Justify = 3
};

struct TextLine {
    std::string text;
    double width = 0.0;
    double x_offset = 0.0;
    double y_offset = 0.0;
};

struct TextLayoutResult {
    std::vector<TextLine> lines;
    double bounding_width = 0.0;
    double bounding_height = 0.0;
};

class TextLayoutEngine {
public:
    // Performs word-wrapping line breaking given an approximate character advance or max width
    static std::vector<std::string> break_lines(
        std::string_view text,
        double max_width,
        double avg_char_width
    );

    // Computes complete multiline layout metrics and line positioning
    static TextLayoutResult compute_layout(
        std::string_view text,
        double max_width,
        double font_size,
        double line_height_ratio,
        TextAlign align
    );
};

} // namespace opencut::text
