#pragma once

#include "core/ids/Ids.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace catchim::media {

struct WaveformBucket {
    float minPeak{0.0f};
    float maxPeak{0.0f};
};

struct WaveformData {
    core::MediaId mediaId;
    int32_t sampleRate{44100};
    int32_t samplesPerBucket{512};
    std::vector<WaveformBucket> buckets;
};

class WaveformGenerator {
public:
    static WaveformGenerator& instance();

    std::shared_ptr<const WaveformData> getWaveform(const core::MediaId& mediaId);
    void cacheWaveform(const core::MediaId& mediaId, std::shared_ptr<WaveformData> data);
    void clear();

    static std::shared_ptr<WaveformData> generateDummyWaveform(const core::MediaId& mediaId, size_t bucketCount = 200);

private:
    WaveformGenerator() = default;
    std::mutex mutex_;
    std::unordered_map<core::MediaId, std::shared_ptr<WaveformData>> cache_;
};

} // namespace catchim::media
