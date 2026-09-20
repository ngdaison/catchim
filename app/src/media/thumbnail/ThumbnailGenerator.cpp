#include "ThumbnailGenerator.h"
#include <format>

namespace catchim::media {

ThumbnailGenerator& ThumbnailGenerator::instance() {
    static ThumbnailGenerator s_instance;
    return s_instance;
}

std::shared_ptr<const ThumbnailImage> ThumbnailGenerator::getThumbnail(
    const core::MediaId& mediaId,
    core::TimelineTime time
) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Round to nearest half-second for caching
    int64_t bucketTime = (time.ticks() / (core::TICKS_PER_SECOND / 2)) * (core::TICKS_PER_SECOND / 2);
    std::string key = std::format("{}:{}", mediaId.str(), bucketTime);

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return it->second;
    }
    return nullptr;
}

void ThumbnailGenerator::cacheThumbnail(
    const core::MediaId& mediaId,
    core::TimelineTime time,
    std::shared_ptr<ThumbnailImage> image
) {
    std::lock_guard<std::mutex> lock(mutex_);
    int64_t bucketTime = (time.ticks() / (core::TICKS_PER_SECOND / 2)) * (core::TICKS_PER_SECOND / 2);
    std::string key = std::format("{}:{}", mediaId.str(), bucketTime);
    cache_[key] = std::move(image);
}

void ThumbnailGenerator::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

std::shared_ptr<ThumbnailImage> ThumbnailGenerator::createColorThumbnail(
    int32_t width,
    int32_t height,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    auto img = std::make_shared<ThumbnailImage>();
    img->width = width;
    img->height = height;
    img->rgbaPixels.resize(width * height * 4);

    for (size_t i = 0; i < img->rgbaPixels.size(); i += 4) {
        img->rgbaPixels[i + 0] = r;
        img->rgbaPixels[i + 1] = g;
        img->rgbaPixels[i + 2] = b;
        img->rgbaPixels[i + 3] = 255;
    }
    return img;
}

} // namespace catchim::media
