#include "editor/subtitles/AssSubtitleParser.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <regex>

namespace catchim::editor {

namespace {

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return str;
}

std::vector<std::string> splitFields(const std::string& line, size_t maxFields = 0) {
    std::vector<std::string> result;
    std::string current;
    for (size_t i = 0; i < line.size(); ++i) {
        if (maxFields > 0 && result.size() + 1 == maxFields) {
            // Last field takes the remainder of the line
            result.push_back(trim(line.substr(i)));
            return result;
        }
        if (line[i] == ',') {
            result.push_back(trim(current));
            current.clear();
        } else {
            current += line[i];
        }
    }
    result.push_back(trim(current));
    return result;
}

} // namespace

double AssSubtitleParser::parseTimestamp(const std::string& input) {
    std::string s = trim(input);
    if (s.empty()) return -1.0;

    // Expected format: H:MM:SS.CC or HH:MM:SS.CC or H:MM:SS.MMM
    int hours = 0;
    int minutes = 0;
    double seconds = 0.0;

    auto colon1 = s.find(':');
    if (colon1 == std::string::npos) return -1.0;
    auto colon2 = s.find(':', colon1 + 1);
    if (colon2 == std::string::npos) return -1.0;

    try {
        hours = std::stoi(s.substr(0, colon1));
        minutes = std::stoi(s.substr(colon1 + 1, colon2 - colon1 - 1));
        seconds = std::stod(s.substr(colon2 + 1));
    } catch (...) {
        return -1.0;
    }

    if (hours < 0 || minutes < 0 || minutes >= 60 || seconds < 0.0 || seconds >= 60.0) {
        // Tolerant if seconds is valid number
        if (hours < 0 || minutes < 0 || seconds < 0.0) return -1.0;
    }

    return static_cast<double>(hours) * 3600.0 + static_cast<double>(minutes) * 60.0 + seconds;
}

std::string AssSubtitleParser::formatTimestamp(double totalSeconds) {
    if (totalSeconds < 0.0) totalSeconds = 0.0;
    int hours = static_cast<int>(totalSeconds / 3600.0);
    double remainder = totalSeconds - hours * 3600.0;
    int minutes = static_cast<int>(remainder / 60.0);
    remainder -= minutes * 60.0;
    int seconds = static_cast<int>(remainder);
    int centis = static_cast<int>((remainder - seconds) * 100.0 + 0.5);
    if (centis >= 100) {
        centis -= 100;
        seconds += 1;
    }

    std::ostringstream oss;
    oss << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds << "."
        << std::setw(2) << std::setfill('0') << centis;
    return oss.str();
}

std::pair<std::string, double> AssSubtitleParser::parseColor(const std::string& assColor) {
    std::string s = trim(assColor);
    if (s.empty()) {
        return {"#ffffff", 1.0};
    }

    // ASS format: &H[AA]BBGGRR or &HAABBGGRR or integer
    uint32_t val = 0;
    if (s.rfind("&H", 0) == 0 || s.rfind("&h", 0) == 0) {
        std::string hexStr = s.substr(2);
        // Remove trailing & if present
        if (!hexStr.empty() && hexStr.back() == '&') {
            hexStr.pop_back();
        }
        try {
            val = static_cast<uint32_t>(std::stoul(hexStr, nullptr, 16));
        } catch (...) {
            return {"#ffffff", 1.0};
        }
    } else {
        try {
            val = static_cast<uint32_t>(std::stoul(s));
        } catch (...) {
            return {"#ffffff", 1.0};
        }
    }

    // Byte order in ASS integer: 0xAABBGGRR or 0xBBGGRR
    uint8_t r = static_cast<uint8_t>(val & 0xFF);
    uint8_t g = static_cast<uint8_t>((val >> 8) & 0xFF);
    uint8_t b = static_cast<uint8_t>((val >> 16) & 0xFF);
    uint8_t a = static_cast<uint8_t>((val >> 24) & 0xFF); // In ASS, 0x00 is opaque, 0xFF is transparent

    double opacity = 1.0 - (static_cast<double>(a) / 255.0);
    if (opacity < 0.0) opacity = 0.0;
    if (opacity > 1.0) opacity = 1.0;

    std::ostringstream oss;
    oss << "#"
        << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(r)
        << std::setw(2) << static_cast<int>(g)
        << std::setw(2) << static_cast<int>(b);

    return {oss.str(), opacity};
}

std::string AssSubtitleParser::stripAssText(const std::string& input, bool& outHadInlineTags) {
    outHadInlineTags = false;
    std::string result;
    result.reserve(input.size());

    bool inTag = false;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '{') {
            inTag = true;
            outHadInlineTags = true;
            continue;
        }
        if (inTag) {
            if (input[i] == '}') {
                inTag = false;
            }
            continue;
        }

        // Check for escaped line breaks \N, \n, \h
        if (input[i] == '\\' && i + 1 < input.size()) {
            char next = input[i + 1];
            if (next == 'N' || next == 'n') {
                result += '\n';
                ++i;
                continue;
            }
            if (next == 'h') {
                result += ' ';
                ++i;
                continue;
            }
        }

        result += input[i];
    }

    return trim(result);
}

ParseAssResult AssSubtitleParser::parse(const std::string& input) {
    ParseAssResult result;
    if (input.empty()) {
        return result;
    }

    double playResX = DEFAULT_PLAY_RES_X;
    double playResY = DEFAULT_PLAY_RES_Y;

    std::unordered_map<std::string, SubtitleStyle> styles;
    std::vector<std::string> styleFormat;
    std::vector<std::string> eventFormat;

    std::string currentSection;
    std::istringstream stream(input);
    std::string line;

    while (std::getline(stream, line)) {
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine[0] == ';') {
            continue;
        }

        // Check section header [Section Name]
        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            currentSection = toLower(trimmedLine.substr(1, trimmedLine.size() - 2));
            continue;
        }

        if (currentSection == "script info") {
            auto colonPos = trimmedLine.find(':');
            if (colonPos != std::string::npos) {
                std::string key = toLower(trim(trimmedLine.substr(0, colonPos)));
                std::string val = trim(trimmedLine.substr(colonPos + 1));
                try {
                    double num = std::stod(val);
                    if (num > 0.0) {
                        if (key == "playresx") playResX = num;
                        else if (key == "playresy") playResY = num;
                    }
                } catch (...) {}
            }
            continue;
        }

        if (currentSection == "v4+ styles" || currentSection == "v4 styles") {
            if (trimmedLine.rfind("Format:", 0) == 0) {
                styleFormat = splitFields(trimmedLine.substr(7));
                for (auto& f : styleFormat) f = toLower(f);
                continue;
            }

            if (trimmedLine.rfind("Style:", 0) == 0 && !styleFormat.empty()) {
                auto values = splitFields(trimmedLine.substr(6), styleFormat.size());
                if (values.size() == styleFormat.size()) {
                    SubtitleStyle style;
                    for (size_t i = 0; i < styleFormat.size(); ++i) {
                        const auto& key = styleFormat[i];
                        const auto& val = values[i];

                        if (key == "name") {
                            style.name = val;
                        } else if (key == "fontname") {
                            style.fontFamily = val;
                        } else if (key == "fontsize") {
                            try {
                                style.fontSize = std::stod(val);
                                style.fontSizeRatioOfPlayHeight = style.fontSize / playResY;
                            } catch (...) {}
                        } else if (key == "primarycolour") {
                            auto [c, op] = parseColor(val);
                            style.color = c;
                            style.opacity = op;
                        } else if (key == "bold") {
                            style.bold = (val == "1" || val == "-1" || toLower(val) == "true");
                        } else if (key == "italic") {
                            style.italic = (val == "1" || val == "-1" || toLower(val) == "true");
                        } else if (key == "underline") {
                            style.underline = (val == "1" || val == "-1" || toLower(val) == "true");
                        } else if (key == "outline") {
                            try { style.outlineWidth = std::stod(val); } catch (...) {}
                        } else if (key == "outlinecolour") {
                            auto [c, op] = parseColor(val);
                            style.outlineColor = c;
                        } else if (key == "shadow") {
                            try { style.shadowWidth = std::stod(val); } catch (...) {}
                        } else if (key == "alignment") {
                            try {
                                int align = std::stoi(val);
                                switch (align) {
                                    case 1:
                                        style.textAlign = SubtitleTextAlign::Left;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Bottom;
                                        break;
                                    case 2:
                                        style.textAlign = SubtitleTextAlign::Center;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Bottom;
                                        break;
                                    case 3:
                                        style.textAlign = SubtitleTextAlign::Right;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Bottom;
                                        break;
                                    case 4:
                                        style.textAlign = SubtitleTextAlign::Left;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Middle;
                                        break;
                                    case 5:
                                        style.textAlign = SubtitleTextAlign::Center;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Middle;
                                        break;
                                    case 6:
                                        style.textAlign = SubtitleTextAlign::Right;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Middle;
                                        break;
                                    case 7:
                                        style.textAlign = SubtitleTextAlign::Left;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Top;
                                        break;
                                    case 8:
                                        style.textAlign = SubtitleTextAlign::Center;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Top;
                                        break;
                                    case 9:
                                        style.textAlign = SubtitleTextAlign::Right;
                                        style.placement.verticalAlign = SubtitleVerticalAlign::Top;
                                        break;
                                    default:
                                        break;
                                }
                            } catch (...) {}
                        } else if (key == "marginl") {
                            try {
                                double ml = std::stod(val);
                                style.placement.marginLeftRatio = ml / playResX;
                            } catch (...) {}
                        } else if (key == "marginr") {
                            try {
                                double mr = std::stod(val);
                                style.placement.marginRightRatio = mr / playResX;
                            } catch (...) {}
                        } else if (key == "marginv") {
                            try {
                                double mv = std::stod(val);
                                style.placement.marginVerticalRatio = mv / playResY;
                            } catch (...) {}
                        }
                    }
                    styles[toLower(style.name)] = style;
                }
            }
            continue;
        }

        if (currentSection == "events") {
            if (trimmedLine.rfind("Format:", 0) == 0) {
                eventFormat = splitFields(trimmedLine.substr(7));
                for (auto& f : eventFormat) f = toLower(f);
                continue;
            }

            if (trimmedLine.rfind("Dialogue:", 0) == 0 && !eventFormat.empty()) {
                auto values = splitFields(trimmedLine.substr(9), eventFormat.size());
                if (values.size() != eventFormat.size()) {
                    result.skippedCueCount++;
                    continue;
                }

                std::string startTimeStr;
                std::string endTimeStr;
                std::string styleName = "default";
                std::string effect;
                std::string rawText;

                for (size_t i = 0; i < eventFormat.size(); ++i) {
                    const auto& key = eventFormat[i];
                    const auto& val = values[i];
                    if (key == "start") startTimeStr = val;
                    else if (key == "end") endTimeStr = val;
                    else if (key == "style") styleName = toLower(val);
                    else if (key == "effect") effect = val;
                    else if (key == "text") rawText = val;
                }

                double startSec = parseTimestamp(startTimeStr);
                double endSec = parseTimestamp(endTimeStr);
                double durationSec = endSec - startSec;

                if (startSec < 0.0 || endSec < 0.0 || durationSec <= 0.0) {
                    result.skippedCueCount++;
                    continue;
                }

                bool hadInlineTags = false;
                std::string cleanedText = stripAssText(rawText, hadInlineTags);
                if (hadInlineTags) {
                    result.strippedInlineTagCueCount++;
                }

                if (cleanedText.empty()) {
                    result.skippedCueCount++;
                    continue;
                }

                if (!effect.empty()) {
                    result.ignoredEffectCount++;
                }

                SubtitleStyle cueStyle;
                auto it = styles.find(styleName);
                if (it != styles.end()) {
                    cueStyle = it->second;
                } else {
                    auto defIt = styles.find("default");
                    if (defIt != styles.end()) {
                        cueStyle = defIt->second;
                    }
                    if (styleName != "default") {
                        result.missingStyleCueCount++;
                    }
                }

                AssSubtitleCue cue;
                cue.text = std::move(cleanedText);
                cue.startTime = core::TimelineTime::fromSeconds(startSec);
                cue.duration = core::TimelineTime::fromSeconds(durationSec);
                cue.style = std::move(cueStyle);

                result.cues.push_back(std::move(cue));
            }
            continue;
        }
    }

    if (result.strippedInlineTagCueCount > 0) {
        result.warnings.push_back("Stripped unsupported ASS inline override tags from " +
            std::to_string(result.strippedInlineTagCueCount) + " subtitle cue(s).");
    }
    if (result.ignoredEffectCount > 0) {
        result.warnings.push_back("Ignored ASS event effects in " +
            std::to_string(result.ignoredEffectCount) + " subtitle cue(s).");
    }
    if (result.missingStyleCueCount > 0) {
        result.warnings.push_back("Fell back to default subtitle styling for " +
            std::to_string(result.missingStyleCueCount) + " cue(s) with missing styles.");
    }

    return result;
}

} // namespace catchim::editor
