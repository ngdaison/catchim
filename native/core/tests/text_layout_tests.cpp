#include "opencut/text_layout.hpp"
#include <cassert>
#include <iostream>

using namespace opencut::text;

static void test_break_lines() {
    const std::string text = "Catchim is a fast video editor running completely on the client browser";
    // Break with max_width of 150 px, avg char width of 10 px (about 15 chars per line)
    auto lines = TextLayoutEngine::break_lines(text, 150.0, 10.0);
    assert(lines.size() >= 4);

    for (const auto& line : lines) {
        assert(line.length() * 10.0 <= 150.0);
    }

    std::cout << "[PASS] test_break_lines\n";
}

static void test_compute_layout_center() {
    const std::string text = "Line One\nMuch Longer Line Two";
    auto layout = TextLayoutEngine::compute_layout(text, 400.0, 20.0, 1.2, TextAlign::Center);

    assert(layout.lines.size() == 2);
    assert(layout.bounding_height == 2 * 20.0 * 1.2);
    // Line 1 is shorter, so it should have x_offset > 0 for center alignment
    assert(layout.lines[0].x_offset > layout.lines[1].x_offset);

    std::cout << "[PASS] test_compute_layout_center\n";
}

int main() {
    test_break_lines();
    test_compute_layout_center();
    std::cout << "All text layout tests passed successfully!\n";
    return 0;
}
