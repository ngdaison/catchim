#include "MediaAsset.h"

namespace catchim::media {

MediaAsset::MediaAsset(
    core::MediaId id,
    std::filesystem::path filePath,
    MediaType type
)
    : id_(std::move(id))
    , filePath_(std::move(filePath))
    , type_(type)
{
    std::error_code ec;
    fileSize_ = std::filesystem::file_size(filePath_, ec);
}

} // namespace catchim::media
