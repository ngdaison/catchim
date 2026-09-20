#include "media/MediaAssetInspector.h"
#include "storage/StorageQuotaEngine.h"
#include <algorithm>
#include <cmath>
#include <cctype>

namespace catchim::media {

namespace {

std::string toLowerString(std::string_view sv) {
    std::string result;
    result.reserve(sv.size());
    for (char c : sv) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return result;
}

std::string toUpperString(std::string_view sv) {
    std::string result;
    result.reserve(sv.size());
    for (char c : sv) {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
    }
    return result;
}

} // namespace

bool MediaAssetInspector::mediaSupportsAudio(MediaType type) noexcept {
    return type == MediaType::Audio || type == MediaType::Video;
}

std::optional<MediaType> MediaAssetInspector::getMediaTypeFromMimeType(std::string_view mimeType) noexcept {
    std::string lower = toLowerString(mimeType);
    if (lower.rfind("image/", 0) == 0) {
        return MediaType::Image;
    }
    if (lower.rfind("video/", 0) == 0) {
        return MediaType::Video;
    }
    if (lower.rfind("audio/", 0) == 0) {
        return MediaType::Audio;
    }
    return std::nullopt;
}

std::optional<MediaType> MediaAssetInspector::getMediaTypeFromExtension(std::string_view extension) noexcept {
    std::string ext = toLowerString(extension);
    if (!ext.empty() && ext.front() == '.') {
        ext.erase(0, 1);
    }

    // Video extensions
    if (ext == "mp4" || ext == "mov" || ext == "webm" || ext == "mkv" || ext == "avi" || ext == "m4v") {
        return MediaType::Video;
    }

    // Audio extensions
    if (ext == "mp3" || ext == "wav" || ext == "aac" || ext == "flac" || ext == "ogg" || ext == "m4a" || ext == "opus") {
        return MediaType::Audio;
    }

    // Image extensions
    if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "webp" || ext == "bmp" || ext == "gif" || ext == "svg") {
        return MediaType::Image;
    }

    return std::nullopt;
}

ThumbnailDimensions MediaAssetInspector::calculateThumbnailSize(int width, int height) noexcept {
    if (width <= 0 || height <= 0) {
        return ThumbnailDimensions{0, 0};
    }

    double aspectRatio = static_cast<double>(width) / static_cast<double>(height);
    int targetWidth = width;
    int targetHeight = height;

    if (targetWidth > kThumbnailMaxWidth) {
        targetWidth = kThumbnailMaxWidth;
        targetHeight = static_cast<int>(std::round(static_cast<double>(targetWidth) / aspectRatio));
    }
    if (targetHeight > kThumbnailMaxHeight) {
        targetHeight = kThumbnailMaxHeight;
        targetWidth = static_cast<int>(std::round(static_cast<double>(targetHeight) * aspectRatio));
    }

    return ThumbnailDimensions{targetWidth, targetHeight};
}

std::string MediaAssetInspector::getUnsupportedVideoDescription(std::string_view codec) {
    std::string lowerCodec = toLowerString(codec);
    std::string codecLabel = codec.empty() ? "this video codec" : toUpperString(codec);

    if (lowerCodec == "hevc" || lowerCodec == "h265") {
        return "HEVC cannot be decoded in this browser, so this clip may not preview correctly. Convert it to H.264 MP4 or try importing it in Safari.";
    }

    return codecLabel + " cannot be decoded in this browser, so this clip may not preview correctly. Convert it to H.264 MP4 and reimport it.";
}

std::string MediaAssetInspector::getStorageLimitDescription(
    uint64_t fileSize,
    std::optional<uint64_t> availableBytes
) {
    std::string fileSizeLabel = storage::StorageQuotaEngine::formatStorageBytes(fileSize);

    if (!availableBytes.has_value()) {
        return "File size is " + fileSizeLabel + ".";
    }

    return "File size is " + fileSizeLabel + ", but only " +
           storage::StorageQuotaEngine::formatStorageBytes(*availableBytes) +
           " is safely available in browser storage.";
}

} // namespace catchim::media
