#include "subtitles/SrtParser.h"
#include <sstream>
#include <algorithm>
#include <charconv>

namespace catchim::subtitles {

static std::string trim(std::string_view sv) {
    while (!sv.empty() && (sv.front() == ' ' || sv.front() == '\t' || sv.front() == '\r' || sv.front() == '\n')) {
        sv.remove_prefix(1);
    }
    while (!sv.empty() && (sv.back() == ' ' || sv.back() == '\t' || sv.back() == '\r' || sv.back() == '\n')) {
        sv.remove_suffix(1);
    }
    return std::string(sv);
}

std::optional<core::TimelineTime> SrtParser::parseTimestamp(std::string_view ts) {
    std::string s = trim(ts);
    if (s.empty()) return std::nullopt;

    // Expected format: HH:MM:SS,mmm or HH:MM:SS.mmm
    size_t firstColon = s.find(':');
    if (firstColon == std::string::npos) return std::nullopt;
    size_t secondColon = s.find(':', firstColon + 1);
    if (secondColon == std::string::npos) return std::nullopt;

    size_t sep = s.find_first_of(",.", secondColon + 1);
    if (sep == std::string::npos) return std::nullopt;

    try {
        int64_t hours = std::stoll(s.substr(0, firstColon));
        int64_t minutes = std::stoll(s.substr(firstColon + 1, secondColon - firstColon - 1));
        int64_t seconds = std::stoll(s.substr(secondColon + 1, sep - secondColon - 1));
        
        std::string msStr = s.substr(sep + 1);
        while (msStr.size() < 3) msStr.push_back('0');
        if (msStr.size() > 3) msStr = msStr.substr(0, 3);
        int64_t millis = std::stoll(msStr);

        if (hours < 0 || minutes < 0 || minutes >= 60 || seconds < 0 || seconds >= 60 || millis < 0 || millis >= 1000) {
            return std::nullopt;
        }

        int64_t ticks = hours * 3600LL * 120000LL
                      + minutes * 60LL * 120000LL
                      + seconds * 120000LL
                      + millis * 120LL;

        return core::TimelineTime(ticks);
    } catch (...) {
        return std::nullopt;
    }
}

ParseResult SrtParser::parse(std::string_view input) {
    ParseResult result;
    if (input.empty()) return result;

    // Normalize newlines to \n
    std::string normalized;
    normalized.reserve(input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\r') {
            if (i + 1 < input.size() && input[i + 1] == '\n') {
                continue; // Skip \r in \r\n
            }
            normalized.push_back('\n');
        } else {
            normalized.push_back(input[i]);
        }
    }

    // Split by double newlines (\n\n+)
    std::vector<std::string> blocks;
    size_t pos = 0;
    while (pos < normalized.size()) {
        size_t next = normalized.find("\n\n", pos);
        if (next == std::string::npos) {
            std::string block = trim(std::string_view(normalized).substr(pos));
            if (!block.empty()) blocks.push_back(std::move(block));
            break;
        }
        std::string block = trim(std::string_view(normalized).substr(pos, next - pos));
        if (!block.empty()) blocks.push_back(std::move(block));
        // Skip consecutive newlines
        pos = next;
        while (pos < normalized.size() && normalized[pos] == '\n') {
            pos++;
        }
    }

    int32_t autoIndex = 1;
    for (const auto& block : blocks) {
        std::istringstream stream(block);
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(stream, line)) {
            std::string trimmed = trim(line);
            if (!trimmed.empty()) {
                lines.push_back(std::move(trimmed));
            }
        }

        if (lines.size() < 2) {
            result.skippedCount++;
            continue;
        }

        // Determine if first line is index or timestamp
        size_t tsLineIdx = 0;
        int32_t cueIdx = autoIndex;
        if (lines[0].find("-->") != std::string::npos) {
            tsLineIdx = 0;
        } else if (lines.size() >= 2 && lines[1].find("-->") != std::string::npos) {
            tsLineIdx = 1;
            try {
                cueIdx = std::stoi(lines[0]);
            } catch (...) {
                cueIdx = autoIndex;
            }
        } else {
            result.skippedCount++;
            continue;
        }

        // Parse timestamp line: start --> end
        const std::string& tsLine = lines[tsLineIdx];
        size_t arrowPos = tsLine.find("-->");
        if (arrowPos == std::string::npos) {
            result.skippedCount++;
            continue;
        }

        std::string startStr = trim(tsLine.substr(0, arrowPos));
        std::string endStr = trim(tsLine.substr(arrowPos + 3));

        auto startOpt = parseTimestamp(startStr);
        auto endOpt = parseTimestamp(endStr);

        if (!startOpt.has_value() || !endOpt.has_value()) {
            result.skippedCount++;
            continue;
        }

        core::TimelineTime start = *startOpt;
        core::TimelineTime end = *endOpt;

        if (end <= start) {
            result.skippedCount++;
            continue;
        }

        // Text lines
        std::string text;
        for (size_t i = tsLineIdx + 1; i < lines.size(); ++i) {
            if (!text.empty()) text.push_back('\n');
            text.append(lines[i]);
        }

        if (text.empty()) {
            result.skippedCount++;
            continue;
        }

        SubtitleCue cue;
        cue.index = cueIdx;
        cue.startTime = start;
        cue.duration = end - start;
        cue.text = std::move(text);

        result.cues.push_back(std::move(cue));
        autoIndex++;
    }

    return result;
}

} // namespace catchim::subtitles
