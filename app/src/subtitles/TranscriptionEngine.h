#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace catchim::subtitles {

struct TranscriptionSegment {
    std::string text;
    double start{0.0};
    double end{0.0};

    TranscriptionSegment() = default;
    TranscriptionSegment(std::string t, double s, double e)
        : text(std::move(t)), start(s), end(e) {}
};

struct CaptionChunk {
    std::string text;
    double startTime{0.0};
    double duration{0.0};

    CaptionChunk() = default;
    CaptionChunk(std::string t, double s, double d)
        : text(std::move(t)), startTime(s), duration(d) {}

    bool operator==(const CaptionChunk& other) const noexcept {
        return text == other.text &&
               startTime == other.startTime &&
               duration == other.duration;
    }
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
