#pragma once
// Feature 222 -- mirrors web/src/media/processing.ts
#include "media/MediaTypeUtils.h"
#include <string>
#include <optional>
#include <vector>
#include <functional>

namespace catchim::media {

struct ProcessedMediaAsset {
    std::string name;
    MediaType type{MediaType::Video};
    std::string url;
    std::optional<std::string> thumbnailUrl;
    std::optional<double> duration;
    std::optional<int> width;
    std::optional<int> height;
    std::optional<int> fps;
    std::optional<bool> hasAudio;
};

struct MediaProcessingResult {
    bool success{false};
    std::string errorMessage;
    std::optional<ProcessedMediaAsset> asset;
};

class MediaProcessingPipeline {
public:
    static std::string getUnsupportedVideoDescription(const std::string& codec);
    static std::string getStorageLimitDescription(long long fileSize,
                                                  std::optional<long long> availableBytes);

    static bool isVideoCodecSupported(const std::string& codec);

    static MediaProcessingResult validateAndPrepare(
        const std::string& fileName,
        const std::string& mimeType,
        long long fileSize,
        std::optional<long long> availableBytes
    );
};

} // namespace catchim::media
