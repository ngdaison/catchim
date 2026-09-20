#include "WaveformCache.h"

namespace catchim::media {

WaveformCache& WaveformCache::instance() {
    static WaveformCache s_instance;
    return s_instance;
}

bool WaveformCache::has(const std::string& sourceKey) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.find(sourceKey) != cache_.end();
}

std::optional<SourceWaveformSummary> WaveformCache::get(const std::string& sourceKey) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(sourceKey);
    if (it != cache_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void WaveformCache::put(const std::string& sourceKey, SourceWaveformSummary summary) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_[sourceKey] = std::move(summary);
}

void WaveformCache::clearSource(const std::string& sourceKey) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.erase(sourceKey);
}

void WaveformCache::clearAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

size_t WaveformCache::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

} // namespace catchim::media
