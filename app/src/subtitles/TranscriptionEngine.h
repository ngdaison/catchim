#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace catchim::subtitles {

struct TranscriptionSegment {
    std::string text;
    double start{0.0};
    double end{0.0};
};

struct CaptionChunk {
    std::string text;
    double startTime{0.0};
    double duration{0.0};
};

class TranscriptionEngine {
public:
    static constexpr size_t DEFAULT_WORDS_PER_CAPTION = 3;
    static constexpr double MIN_CAPTION_DURATION_SECONDS = 0.8;

    static std::vector<CaptionChunk> buildCaptionChunks(
        const std::vector<TranscriptionSegment>& segments,
        size_t wordsPerChunk = DEFAULT_WORDS_PER_CAPTION,
        double minDuration = MIN_CAPTION_DURATION_SECONDS
    );

    static std::vector<TranscriptionSegment> parseWhisperJson(const std::string& jsonStr);
};

} // namespace catchim::subtitles
