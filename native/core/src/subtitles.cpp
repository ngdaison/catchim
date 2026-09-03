#define _CRT_SECURE_NO_WARNINGS
#include "opencut/subtitles.hpp"
#include "opencut/time.hpp"
#include <cstdio>
#include <cmath>
#include <sstream>
#include <functional>

namespace opencut::subtitles {

int64_t SrtParser::parse_timestamp(std::string_view ts) {
    while (!ts.empty() && (ts.front() == ' ' || ts.front() == '\t')) {
        ts.remove_prefix(1);
    }
    while (!ts.empty() && (ts.back() == ' ' || ts.back() == '\t' || ts.back() == '\r')) {
        ts.remove_suffix(1);
    }

    int h = 0, m = 0, s = 0, ms = 0;
    std::string str(ts);
    char sep = ',';
    if (std::sscanf(str.c_str(), "%d:%d:%d%c%d", &h, &m, &s, &sep, &ms) >= 4) {
        double total_sec = h * 3600.0 + m * 60.0 + s + (ms / 1000.0);
        auto mt = MediaTime::from_seconds_f64(total_sec);
        return mt ? mt->as_ticks() : static_cast<int64_t>(std::round(total_sec * 120000.0));
    }
    return 0;
}

std::string SrtParser::format_timestamp(int64_t ticks) {
    double total_sec = MediaTime::from_ticks(ticks).to_seconds_f64();
    if (total_sec < 0.0) total_sec = 0.0;

    int total_ms = static_cast<int>(std::round(total_sec * 1000.0));
    int ms = total_ms % 1000;
    int total_s = total_ms / 1000;
    int s = total_s % 60;
    int total_m = total_s / 60;
    int m = total_m % 60;
    int h = total_m / 60;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d,%03d", h, m, s, ms);
    return std::string(buf);
}

std::vector<SubtitleCue> SrtParser::parse(std::string_view srt_content) {
    std::vector<SubtitleCue> cues;
    std::string content(srt_content);

    // Normalize \r\n to \n
    std::string normalized;
    normalized.reserve(content.size());
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] == '\r') {
            if (i + 1 < content.size() && content[i + 1] == '\n') {
                continue;
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(content[i]);
        }
    }

    std::istringstream stream(normalized);
    std::string line;
    std::vector<std::string> current_block;

    auto process_block = [&cues](const std::vector<std::string>& block) {
        if (block.size() < 2) return;

        size_t time_idx = 0;
        int32_t idx = 1;
        if (block[0].find("-->") == std::string::npos) {
            std::sscanf(block[0].c_str(), "%d", &idx);
            time_idx = 1;
        }

        if (time_idx >= block.size()) return;

        const auto& time_line = block[time_idx];
        auto arrow_pos = time_line.find("-->");
        if (arrow_pos == std::string::npos) return;

        std::string_view start_str(time_line.data(), arrow_pos);
        std::string_view end_str(time_line.data() + arrow_pos + 3, time_line.size() - arrow_pos - 3);

        int64_t start_ticks = parse_timestamp(start_str);
        int64_t end_ticks = parse_timestamp(end_str);

        if (end_ticks <= start_ticks) return;

        std::string text;
        for (size_t i = time_idx + 1; i < block.size(); ++i) {
            if (!text.empty()) text += "\n";
            text += block[i];
        }

        if (text.empty()) return;

        cues.push_back(SubtitleCue{
            .index = idx,
            .start_time = start_ticks,
            .duration = end_ticks - start_ticks,
            .text = std::move(text)
        });
    };

    while (std::getline(stream, line)) {
        while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t')) {
            line.pop_back();
        }

        if (line.empty()) {
            if (!current_block.empty()) {
                process_block(current_block);
                current_block.clear();
            }
        } else {
            current_block.push_back(line);
        }
    }

    if (!current_block.empty()) {
        process_block(current_block);
    }

    return cues;
}

std::string SrtParser::format(const std::vector<SubtitleCue>& cues) {
    std::ostringstream out;
    for (size_t i = 0; i < cues.size(); ++i) {
        const auto& cue = cues[i];
        out << (i + 1) << "\n";
        out << format_timestamp(cue.start_time) << " --> " << format_timestamp(cue.end_time()) << "\n";
        out << cue.text << "\n\n";
    }
    return out.str();
}

} // namespace opencut::subtitles
