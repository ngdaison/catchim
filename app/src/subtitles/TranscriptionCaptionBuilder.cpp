#include "TranscriptionCaptionBuilder.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace catchim::subtitles {

std::vector<std::string> TranscriptionCaptionBuilder::splitWords(const std::string& text) {
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        if (!word.empty()) {
            words.push_back(word);
        }
    }
    return words;
}

std::vector<CaptionChunk> TranscriptionCaptionBuilder::buildCaptionChunks(
    const std::vector<TranscriptionSegment>& segments,
    int wordsPerChunk,
    double minDuration) {

    std::vector<CaptionChunk> captions;
    double globalEndTime = 0.0;
    const int resolvedWordsPerChunk = std::max(1, wordsPerChunk);
    const double resolvedMinDuration = std::max(0.0, minDuration);

    for (const auto& segment : segments) {
        const auto words = splitWords(segment.text);
        if (words.empty()) {
            continue;
        }

        const double segmentDuration = segment.end - segment.start;
        const double wordsPerSecond = (segmentDuration > 0.0)
            ? (static_cast<double>(words.size()) / segmentDuration)
            : 1.0;

        // Group into chunks
        std::vector<std::string> chunks;
        for (size_t i = 0; i < words.size(); i += static_cast<size_t>(resolvedWordsPerChunk)) {
            std::string chunkStr;
            const size_t endIdx = std::min(words.size(), i + static_cast<size_t>(resolvedWordsPerChunk));
            for (size_t j = i; j < endIdx; ++j) {
                if (!chunkStr.empty()) {
                    chunkStr += " ";
                }
                chunkStr += words[j];
            }
            chunks.push_back(std::move(chunkStr));
        }

        double chunkStartTime = segment.start;
        for (const auto& chunk : chunks) {
            const auto chunkWordsList = splitWords(chunk);
            const double chunkWords = static_cast<double>(chunkWordsList.size());
            const double chunkDuration = std::max(
                resolvedMinDuration,
                (wordsPerSecond > 0.0) ? (chunkWords / wordsPerSecond) : resolvedMinDuration);
            const double adjustedStartTime = std::max(chunkStartTime, globalEndTime);

            captions.push_back(CaptionChunk{
                chunk,
                adjustedStartTime,
                chunkDuration
            });

            globalEndTime = adjustedStartTime + chunkDuration;
            chunkStartTime += chunkDuration;
        }
    }

    return captions;
}

} // namespace catchim::subtitles
