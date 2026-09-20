// Feature 222 -- mirrors web/src/media/processing.ts
#include "media/MediaProcessingPipeline.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace {
std::string formatBytes(long long bytes) {
    if (bytes <= 0) return "0 B";
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double val = static_cast<double>(bytes);
    int unitIndex = 0;
    while (val >= 1024.0 && unitIndex < 4) {
        val /= 1024.0;
        unitIndex++;
    }
    std::ostringstream oss;
    if (val >= 10.0 || unitIndex == 0) {
        oss << std::fixed << std::setprecision(0) << val << " " << units[unitIndex];
    } else {
        oss << std::fixed << std::setprecision(1) << val << " " << units[unitIndex];
    }
    return oss.str();
}
} // namespace

namespace catchim::media {

std::string MediaProcessingPipeline::getUnsupportedVideoDescription(const std::string& codec) {
    std::string codecUpper = codec.empty() ? "this video codec" : codec;
    std::transform(codecUpper.begin(), codecUpper.end(), codecUpper.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });

    std::string codecLower = codec;
    std::transform(codecLower.begin(), codecLower.end(), codecLower.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

    if (codecLower == "hevc") {
        return codecUpper + " cannot be decoded in this browser, so this clip may not preview correctly. Convert it to H.264 MP4 or try importing it in Safari.";
    }
    return codecUpper + " cannot be decoded in this browser, so this clip may not preview correctly. Convert it to H.264 MP4 and reimport it.";
}

std::string MediaProcessingPipeline::getStorageLimitDescription(long long fileSize,
                                                              std::optional<long long> availableBytes) {
    std::string fileSizeLabel = formatBytes(fileSize);
    if (!availableBytes.has_value()) {
        return "File size is " + fileSizeLabel + ".";
    }
    return "File size is " + fileSizeLabel + ", but only " + formatBytes(*availableBytes) + " is safely available in browser storage.";
}

bool MediaProcessingPipeline::isVideoCodecSupported(const std::string& codec) {
    std::string c = codec;
    std::transform(c.begin(), c.end(), c.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    // h264, vp8, vp9, av1 are broadly supported
    return (c == "h264" || c == "avc" || c == "avc1" || c == "vp8" || c == "vp9" || c == "av1");
}

MediaProcessingResult MediaProcessingPipeline::validateAndPrepare(
    const std::string& fileName,
    const std::string& mimeType,
    long long fileSize,
    std::optional<long long> availableBytes
) {
    MediaProcessingResult res;
    auto typeOpt = getMediaTypeFromMime(mimeType);
    if (!typeOpt.has_value()) {
        res.success = false;
        res.errorMessage = "Unsupported file type: " + fileName;
        return res;
    }

    if (availableBytes.has_value() && fileSize > *availableBytes) {
        res.success = false;
        res.errorMessage = "Not enough storage for " + fileName + ": " + getStorageLimitDescription(fileSize, availableBytes);
        return res;
    }

    ProcessedMediaAsset asset;
    asset.name = fileName;
    asset.type = *typeOpt;
    asset.url = fileName;
    res.success = true;
    res.asset = asset;
    return res;
}

} // namespace catchim::media
