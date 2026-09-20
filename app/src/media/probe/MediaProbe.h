#pragma once

#include "media/MediaAsset.h"
#include "core/errors/Errors.h"
#include <filesystem>
#include <memory>

namespace catchim::media {

class MediaProbe {
public:
    static core::Result<std::shared_ptr<MediaAsset>> probe(const std::filesystem::path& path);
    static MediaType detectType(const std::filesystem::path& path);
};

} // namespace catchim::media
