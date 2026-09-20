#pragma once

#include "media/MediaAsset.h"
#include "media/MediaAssetInspector.h"
#include "render/compositor/RenderSurface.h"
#include <cstdint>
#include <string>
#include <optional>
#include <filesystem>

namespace catchim::media {

/**
 * @brief Engine for thumbnail dimension calculation, media type detection, and thumbnail surface generation.
 * Corresponds to web/src/media/thumbnail.ts and web/src/media/media-utils.ts.
 */
class MediaThumbnailEngine {
public:
    static constexpr int THUMBNAIL_MAX_WIDTH = 1280;
    static constexpr int THUMBNAIL_MAX_HEIGHT = 720;

    /**
     * @brief Computes constrained thumbnail dimensions preserving aspect ratio within 1280x720.
     * Corresponds to thumbnailSize() in web/src/media/thumbnail.ts.
     */
    static ThumbnailDimensions calculateThumbnailSize(int width, int height) noexcept;

    /**
     * @brief Checks if a media type or asset supports audio.
     * Corresponds to mediaSupportsAudio() in web/src/media/media-utils.ts.
     */
    static bool supportsAudio(MediaType type) noexcept;
    static bool supportsAudio(const MediaAsset* asset) noexcept;

    /**
     * @brief Detects media type from a MIME string (e.g. "video/mp4", "image/png", "audio/mpeg").
     * Corresponds to getMediaTypeFromFile in web/src/media/media-utils.ts.
     */
    static std::optional<MediaType> detectMediaTypeFromMime(const std::string& mime) noexcept;

    /**
     * @brief Detects media type from a file path or extension.
     */
    static std::optional<MediaType> detectMediaTypeFromPath(const std::filesystem::path& path) noexcept;

    /**
     * @brief Detects media type from a MIME type or file path string.
     */
    static std::optional<MediaType> detectMediaType(const std::string& mimeOrPath) noexcept;

    /**
     * @brief Generates a resized thumbnail RenderSurface using bilinear sampling.
     */
    static render::RenderSurface createThumbnailSurface(const render::RenderSurface& src);
};

} // namespace catchim::media
