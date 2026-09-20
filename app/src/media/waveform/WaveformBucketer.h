#pragma once

#include "media/waveform/WaveformGenerator.h"
#include <vector>
#include <memory>
#include <cstddef>
#include <cstdint>

namespace catchim::media {

class WaveformBucketer {
public:
    static std::vector<float> computePeakBuckets(
        const float* samples,
        size_t sampleCount,
        size_t bucketSize
    ) noexcept;

    static std::vector<float> computeMultiChannelPeakBuckets(
        const std::vector<const float*>& channels,
        size_t sampleCount,
        size_t bucketSize
    ) noexcept;

    static std::shared_ptr<WaveformData> generateWaveformData(
        const core::MediaId& mediaId,
        const float* samples,
        size_t sampleCount,
        int32_t sampleRate = 44100,
        int32_t bucketSize = 256
    );
};

} // namespace catchim::media
