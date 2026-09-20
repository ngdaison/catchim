#include "subtitles/TranscriptionEngine.h"
#include <sstream>
#include <algorithm>

namespace catchim::subtitles {

namespace {

std::vector<std::string> splitWords(const std::string& str) {
    std::vector<std::string> words;
    std::istringstream iss(str);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

std::string joinWords(const std::vector<std::string>& words, size_t start, size_t count) {
    std::string result;
    for (size_t i = 0; i < count && (start + i) < words.size(); ++i) {
        if (i > 0) result += " ";
        result += words[start + i];
    }
    return result;
}

} // namespace

std::vector<CaptionChunk> TranscriptionEngine::buildCaptionChunks(
    const std::vector<TranscriptionSegment>& segments,
    size_t wordsPerChunk,
    double minDuration
) {
    if (wordsPerChunk == 0) wordsPerChunk = DEFAULT_WORDS_PER_CAPTION;
    std::vector<CaptionChunk> captions;
    double globalEndTime = 0.0;

    for (const auto& segment : segments) {
        auto words = splitWords(segment.text);
        if (words.empty()) continue;

        double segmentDuration = segment.end - segment.start;
        if (segmentDuration <= 0.0) segmentDuration = 0.1;
        double wordsPerSecond = static_cast<double>(words.size()) / segmentDuration;

        std::vector<std::string> chunks;
        for (size_t i = 0; i < words.size(); i += wordsPerChunk) {
            chunks.push_back(joinWords(words, i, wordsPerChunk));
        }

        double chunkStartTime = segment.start;
        for (const auto& chunk : chunks) {
            auto chunkWords = splitWords(chunk);
            double chunkDuration = std::max(minDuration, static_cast<double>(chunkWords.size()) / wordsPerSecond);
            double adjustedStartTime = std::max(chunkStartTime, globalEndTime);

            captions.push_back({chunk, adjustedStartTime, chunkDuration});

            globalEndTime = adjustedStartTime + chunkDuration;
            chunkStartTime += chunkDuration;
        }
    }

    return captions;
}

std::vector<TranscriptionSegment> TranscriptionEngine::parseWhisperJson(const std::string& jsonStr) {
    std::vector<TranscriptionSegment> result;
    try {
        auto j = nlohmann::json::parse(jsonStr);
        if (j.contains("segments") && j["segments"].is_array()) {
            for (const auto& seg : j["segments"]) {
                TranscriptionSegment s;
                s.text = seg.value("text", "");
                s.start = seg.value("start", 0.0);
                s.end = seg.value("end", 0.0);
                result.push_back(s);
            }
        }
    } catch (...) {}
    return result;
}

} // namespace catchim::subtitles
