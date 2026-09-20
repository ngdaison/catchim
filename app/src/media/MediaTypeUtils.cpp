// Feature 222 -- mirrors web/src/media/media-utils.ts
#include "media/MediaTypeUtils.h"

namespace catchim::media {

bool mediaSupportsAudio(MediaType type) {
    return type == MediaType::Audio || type == MediaType::Video;
}

bool mediaSupportsAudioOpt(const std::optional<MediaType>& type) {
    if (!type.has_value()) return false;
    return mediaSupportsAudio(*type);
}

std::optional<MediaType> getMediaTypeFromMime(const std::string& mimeType) {
    if (mimeType.rfind("image/", 0) == 0) return MediaType::Image;
    if (mimeType.rfind("video/", 0) == 0) return MediaType::Video;
    if (mimeType.rfind("audio/", 0) == 0) return MediaType::Audio;
    return std::nullopt;
}

} // namespace catchim::media
