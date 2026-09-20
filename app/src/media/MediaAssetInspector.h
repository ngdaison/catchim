#pragma once

#include "media/MediaAsset.h"
#include <string>
#include <string_view>
#include <optional>
#include <cstdint>

namespace catchim::media {

struct ThumbnailDimensions {
    int width{0};
    int height{0};

    bool operator==(const ThumbnailDimensions& other) const noexcept = default;
};

class MediaAssetInspector {
public:
    static constexpr int kThumbnailMaxWidth = 1280;
    static constexpr int kThumbnailMaxHeight = 720;

    // Checks whether media type supports audio tracks (Video, Audio)
    static bool mediaSupportsAudio(MediaType type) noexcept;

    // Infers MediaType from MIME string (e.g. "video/mp4" -> MediaType::Video)
    static std::optional<MediaType> getMediaTypeFromMimeType(std::string_view mimeType) noexcept;

    // Infers MediaType from file extension (e.g. ".mp4", "png")
    static std::optional<MediaType> getMediaTypeFromExtension(std::string_view extension) noexcept;

    // Calculates constrained thumbnail bounds preserving aspect ratio (max 1280x720)
    static ThumbnailDimensions calculateThumbnailSize(int width, int height) noexcept;

    // Diagnostic warning text for unsupported codecs
    static std::string getUnsupportedVideoDescription(std::string_view codec);

    // Warning description when file size exceeds safe storage headroom
    static std::string getStorageLimitDescription(
        uint64_t fileSize,
        std::optional<uint64_t> availableBytes = std::nullopt
    );
};

} // namespace catchim::media
