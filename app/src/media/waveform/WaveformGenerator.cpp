#include "WaveformGenerator.h"
#include <cmath>

namespace catchim::media {

WaveformGenerator& WaveformGenerator::instance() {
    static WaveformGenerator s_instance;
    return s_instance;
}

std::shared_ptr<const WaveformData> WaveformGenerator::getWaveform(const core::MediaId& mediaId) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(mediaId);
    if (it != cache_.end()) {
        return it->second;
    }
    return nullptr;
}

void WaveformGenerator::cacheWaveform(const core::MediaId& mediaId, std::shared_ptr<WaveformData> data) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_[mediaId] = std::move(data);
}

void WaveformGenerator::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

std::shared_ptr<WaveformData> WaveformGenerator::generateDummyWaveform(const core::MediaId& mediaId, size_t bucketCount) {
    auto data = std::make_shared<WaveformData>();
    data->mediaId = mediaId;
    data->buckets.reserve(bucketCount);

    for (size_t i = 0; i < bucketCount; ++i) {
        float phase = static_cast<float>(i) * 0.1f;
        float amp = 0.3f + 0.5f * std::abs(std::sin(phase) * std::cos(phase * 0.5f));
        data->buckets.push_back({-amp, amp});
    }

    return data;
}

} // namespace catchim::media
