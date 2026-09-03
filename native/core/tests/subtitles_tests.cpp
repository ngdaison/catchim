#include "opencut/subtitles.hpp"
#include "opencut/time.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace opencut::subtitles;

static void test_parse_srt() {
    const std::string srt = 
        "1\n"
        "00:00:01,000 --> 00:00:04,500\n"
        "Hello world!\n"
        "Second line.\n"
        "\n"
        "2\n"
        "00:00:05,000 --> 00:00:08,200\n"
        "Catchim video editor\n";

    auto cues = SrtParser::parse(srt);
    assert(cues.size() == 2);
    assert(cues[0].index == 1);
    assert(cues[0].text == "Hello world!\nSecond line.");
    assert(cues[1].index == 2);
    assert(cues[1].text == "Catchim video editor");

    // Check timestamps in seconds
    double start1 = opencut::MediaTime::from_ticks(cues[0].start_time).to_seconds_f64();
    double end1 = opencut::MediaTime::from_ticks(cues[0].end_time()).to_seconds_f64();
    assert(std::abs(start1 - 1.0) < 0.001);
    assert(std::abs(end1 - 4.5) < 0.001);

    std::cout << "[PASS] test_parse_srt\n";
}

static void test_format_srt() {
    std::vector<SubtitleCue> cues;
    SubtitleCue cue;
    cue.index = 1;
    cue.start_time = opencut::MediaTime::from_seconds_f64(0.5)->as_ticks();
    cue.duration = opencut::MediaTime::from_seconds_f64(2.0)->as_ticks();
    cue.text = "Testing subtitle formatting";
    cues.push_back(cue);

    std::string formatted = SrtParser::format(cues);
    assert(formatted.find("00:00:00,500 --> 00:00:02,500") != std::string::npos);
    assert(formatted.find("Testing subtitle formatting") != std::string::npos);

    std::cout << "[PASS] test_format_srt\n";
}

int main() {
    test_parse_srt();
    test_format_srt();
    std::cout << "All subtitles tests passed successfully!\n";
    return 0;
}
