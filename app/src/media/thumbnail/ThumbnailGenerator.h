#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace catchim::media {

struct ThumbnailImage {
    int32_t width{120};
    int32_t height{68};
    std::vector<uint8_t> rgbaPixels;
};

class ThumbnailGenerator {
public:
    static ThumbnailGenerator& instance();

    std::shared_ptr<const ThumbnailImage> getThumbnail(
        const core::MediaId& mediaId,
        core::TimelineTime time
    );

    void cacheThumbnail(
        const core::MediaId& mediaId,
        core::TimelineTime time,
        std::shared_ptr<ThumbnailImage> image
    );

    void clear();

    static std::shared_ptr<ThumbnailImage> createColorThumbnail(
        int32_t width,
        int32_t height,
        uint8_t r,
        uint8_t g,
        uint8_t b
    );

private:
    ThumbnailGenerator() = default;
    std::mutex mutex_;
    std::unordered_map<std::string, std::shared_ptr<ThumbnailImage>> cache_;
};

} // namespace catchim::media
