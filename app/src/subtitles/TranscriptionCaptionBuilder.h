#pragma once

#include "TranscriptionEngine.h"
#include <string>
#include <vector>

namespace catchim::subtitles {

class TranscriptionCaptionBuilder {
public:
    static std::vector<CaptionChunk> buildCaptionChunks(
        const std::vector<TranscriptionSegment>& segments,
        int wordsPerChunk = static_cast<int>(TranscriptionEngine::DEFAULT_WORDS_PER_CAPTION),
        double minDuration = TranscriptionEngine::MIN_CAPTION_DURATION_SECONDS);

private:
    static std::vector<std::string> splitWords(const std::string& text);
};

} // namespace catchim::subtitles
