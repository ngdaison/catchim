#pragma once
// Feature 222 -- mirrors web/src/media/media-utils.ts
#include "media/MediaAsset.h"
#include <string>
#include <optional>

namespace catchim::media {

// SUPPORTS_AUDIO: audio and video types
bool mediaSupportsAudio(MediaType type);
bool mediaSupportsAudioOpt(const std::optional<MediaType>& type);

// Detect media type from MIME type string (e.g. "video/mp4" -> MediaType::Video)
std::optional<MediaType> getMediaTypeFromMime(const std::string& mimeType);

} // namespace catchim::media
