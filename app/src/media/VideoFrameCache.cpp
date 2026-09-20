#include "media/VideoFrameCache.h"
#include <algorithm>

namespace catchim::media {

VideoFrameCache::VideoFrameCache(size_t maxFrames, size_t maxMemoryBytes)
    : maxFrames_(maxFrames), maxMemoryBytes_(maxMemoryBytes)
{
}

uint64_t VideoFrameCache::currentGeneration() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentGeneration_;
}

uint64_t VideoFrameCache::incrementGeneration() {
    std::lock_guard<std::mutex> lock(mutex_);
    return ++currentGeneration_;
}

std::shared_ptr<const CachedVideoFrame> VideoFrameCache::getFrameAt(
    const core::MediaId& mediaId,
    core::TimelineTime timestamp,
    core::TimelineTime tolerance
) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::shared_ptr<CachedVideoFrame> bestMatch = nullptr;
    int64_t bestDiffTicks = std::numeric_limits<int64_t>::max();

    for (const auto& frame : frames_) {
        if (frame->mediaId == mediaId) {
            int64_t diff = std::abs((frame->timestamp - timestamp).ticks());
            if (diff <= tolerance.ticks() && diff < bestDiffTicks) {
                bestDiffTicks = diff;
                bestMatch = frame;
            }
        }
    }

    if (bestMatch) {
        bestMatch->accessCounter = ++globalAccessCounter_;
    }

    return bestMatch;
}

void VideoFrameCache::storeFrame(
    const core::MediaId& mediaId,
    core::TimelineTime timestamp,
    core::TimelineTime duration,
    int width,
    int height,
    std::vector<uint8_t> rgbaPixels,
    uint64_t generation
) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (generation > 0 && generation < currentGeneration_) {
        // Discard stale frame generated before a seek
        return;
    }

    auto frame = std::make_shared<CachedVideoFrame>();
    frame->mediaId = mediaId;
    frame->timestamp = timestamp;
    frame->duration = duration;
    frame->width = width;
    frame->height = height;
    frame->rgbaPixels = std::move(rgbaPixels);
    frame->accessCounter = ++globalAccessCounter_;
    frame->generation = generation;

    currentMemoryBytes_ += frame->byteSize();
    frames_.push_back(frame);

    evictIfNeededLocked();
}

void VideoFrameCache::invalidateMedia(const core::MediaId& mediaId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::remove_if(frames_.begin(), frames_.end(), [&](const auto& frame) {
        if (frame->mediaId == mediaId) {
            currentMemoryBytes_ -= std::min(currentMemoryBytes_, frame->byteSize());
            return true;
        }
        return false;
    });
    frames_.erase(it, frames_.end());
}

void VideoFrameCache::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    frames_.clear();
    currentMemoryBytes_ = 0;
}

size_t VideoFrameCache::frameCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_.size();
}

size_t VideoFrameCache::memoryUsageBytes() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return currentMemoryBytes_;
}

void VideoFrameCache::setCapacity(size_t maxFrames, size_t maxMemoryBytes) {
    std::lock_guard<std::mutex> lock(mutex_);
    maxFrames_ = maxFrames;
    maxMemoryBytes_ = maxMemoryBytes;
    evictIfNeededLocked();
}

void VideoFrameCache::evictIfNeededLocked() {
    while (!frames_.empty() && (frames_.size() > maxFrames_ || currentMemoryBytes_ > maxMemoryBytes_)) {
        // Find LRU frame (minimum accessCounter)
        auto lruIt = std::min_element(frames_.begin(), frames_.end(), [](const auto& a, const auto& b) {
            return a->accessCounter < b->accessCounter;
        });

        if (lruIt != frames_.end()) {
            currentMemoryBytes_ -= std::min(currentMemoryBytes_, (*lruIt)->byteSize());
            frames_.erase(lruIt);
        } else {
            break;
        }
    }
}

} // namespace catchim::media
