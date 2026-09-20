#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <cstdint>

namespace catchim::media {

struct CachedVideoFrame {
    core::MediaId mediaId;
    core::TimelineTime timestamp;
    core::TimelineTime duration;
    int width{0};
    int height{0};
    std::vector<uint8_t> rgbaPixels;
    uint64_t accessCounter{0};
    uint64_t generation{0};

    size_t byteSize() const noexcept {
        return rgbaPixels.size() + sizeof(*this);
    }
};

class VideoFrameCache {
public:
    explicit VideoFrameCache(size_t maxFrames = 60, size_t maxMemoryBytes = 256 * 1024 * 1024);

    // Generation tracking for scrub / seek cancellation
    uint64_t currentGeneration() const;
    uint64_t incrementGeneration();

    // Cache access
    std::shared_ptr<const CachedVideoFrame> getFrameAt(
        const core::MediaId& mediaId,
        core::TimelineTime timestamp,
        core::TimelineTime tolerance = core::TimelineTime::fromTicks(4000) // ~1 frame at 30fps
    );

    void storeFrame(
        const core::MediaId& mediaId,
        core::TimelineTime timestamp,
        core::TimelineTime duration,
        int width,
        int height,
        std::vector<uint8_t> rgbaPixels,
        uint64_t generation = 0
    );

    void invalidateMedia(const core::MediaId& mediaId);
    void clear();

    size_t frameCount() const;
    size_t memoryUsageBytes() const;
    size_t maxFrames() const { return maxFrames_; }
    size_t maxMemoryBytes() const { return maxMemoryBytes_; }

    void setCapacity(size_t maxFrames, size_t maxMemoryBytes);

private:
    void evictIfNeededLocked();

    mutable std::mutex mutex_;
    size_t maxFrames_{60};
    size_t maxMemoryBytes_{256 * 1024 * 1024};
    size_t currentMemoryBytes_{0};
    uint64_t globalAccessCounter_{0};
    uint64_t currentGeneration_{1};

    std::vector<std::shared_ptr<CachedVideoFrame>> frames_;
};

} // namespace catchim::media
