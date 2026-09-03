#include "opencut/text_layout.hpp"
#include <sstream>
#include <algorithm>

namespace opencut::text {

std::vector<std::string> TextLayoutEngine::break_lines(
    std::string_view text,
    double max_width,
    double avg_char_width
) {
    std::vector<std::string> result_lines;
    if (text.empty()) {
        return result_lines;
    }

    if (max_width <= 0.0 || avg_char_width <= 0.0) {
        // Just split by newlines
        std::string current;
        for (char c : text) {
            if (c == '\n') {
                result_lines.push_back(std::move(current));
                current.clear();
            } else if (c != '\r') {
                current.push_back(c);
            }
        }
        if (!current.empty() || text.back() == '\n') {
            result_lines.push_back(std::move(current));
        }
        return result_lines;
    }

    std::string str(text);
    std::istringstream para_stream(str);
    std::string para;

    while (std::getline(para_stream, para)) {
        while (!para.empty() && (para.back() == '\r' || para.back() == ' ')) {
            para.pop_back();
        }

        if (para.empty()) {
            result_lines.push_back("");
            continue;
        }

        std::istringstream word_stream(para);
        std::string word;
        std::string current_line;
        double current_w = 0.0;

        while (word_stream >> word) {
            double word_w = word.length() * avg_char_width;
            double space_w = current_line.empty() ? 0.0 : avg_char_width;

            if (current_w + space_w + word_w > max_width && !current_line.empty()) {
                result_lines.push_back(std::move(current_line));
                current_line = word;
                current_w = word_w;
            } else {
                if (!current_line.empty()) {
                    current_line.push_back(' ');
                    current_w += space_w;
                }
                current_line += word;
                current_w += word_w;
            }
        }

        if (!current_line.empty()) {
            result_lines.push_back(std::move(current_line));
        }
    }

    return result_lines;
}

TextLayoutResult TextLayoutEngine::compute_layout(
    std::string_view text,
    double max_width,
    double font_size,
    double line_height_ratio,
    TextAlign align
) {
    TextLayoutResult result;
    if (font_size <= 0.0) font_size = 16.0;
    if (line_height_ratio <= 0.0) line_height_ratio = 1.2;

    const double avg_char_width = font_size * 0.55;
    const double line_height = font_size * line_height_ratio;

    auto raw_lines = break_lines(text, max_width, avg_char_width);
    if (raw_lines.empty()) {
        return result;
    }

    double max_line_w = 0.0;
    std::vector<double> line_widths;
    line_widths.reserve(raw_lines.size());

    for (const auto& line_str : raw_lines) {
        double w = line_str.length() * avg_char_width;
        line_widths.push_back(w);
        max_line_w = std::max(max_line_w, w);
    }

    result.bounding_width = max_width > 0.0 ? std::min(max_width, max_line_w) : max_line_w;
    result.bounding_height = raw_lines.size() * line_height;

    result.lines.reserve(raw_lines.size());
    for (size_t i = 0; i < raw_lines.size(); ++i) {
        double x = 0.0;
        double w = line_widths[i];

        if (align == TextAlign::Center) {
            x = (result.bounding_width - w) * 0.5;
        } else if (align == TextAlign::Right) {
            x = result.bounding_width - w;
        }

        result.lines.push_back(TextLine{
            .text = std::move(raw_lines[i]),
            .width = w,
            .x_offset = x,
            .y_offset = static_cast<double>(i) * line_height
        });
    }

    return result;
}

} // namespace opencut::text
