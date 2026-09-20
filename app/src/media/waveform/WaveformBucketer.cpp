#include "media/waveform/WaveformBucketer.h"
#include <algorithm>
#include <cmath>

namespace catchim::media {

std::vector<float> WaveformBucketer::computePeakBuckets(
    const float* samples,
    size_t sampleCount,
    size_t bucketSize
) noexcept {
    if (!samples || sampleCount == 0 || bucketSize == 0) {
        return {};
    }

    size_t numBuckets = (sampleCount + bucketSize - 1) / bucketSize;
    std::vector<float> peaks(numBuckets, 0.0f);

    for (size_t b = 0; b < numBuckets; ++b) {
        size_t start = b * bucketSize;
        size_t end = std::min(start + bucketSize, sampleCount);

        float peak = 0.0f;
        for (size_t i = start; i < end; ++i) {
            float absVal = std::abs(samples[i]);
            if (absVal > peak) {
                peak = absVal;
            }
        }
        peaks[b] = peak;
    }

    return peaks;
}

std::vector<float> WaveformBucketer::computeMultiChannelPeakBuckets(
    const std::vector<const float*>& channels,
    size_t sampleCount,
    size_t bucketSize
) noexcept {
    if (channels.empty() || sampleCount == 0 || bucketSize == 0) {
        return {};
    }

    size_t numBuckets = (sampleCount + bucketSize - 1) / bucketSize;
    std::vector<float> peaks(numBuckets, 0.0f);

    for (const auto* channel : channels) {
        if (!channel) continue;
        auto chPeaks = computePeakBuckets(channel, sampleCount, bucketSize);
        for (size_t b = 0; b < numBuckets; ++b) {
            if (b < chPeaks.size() && chPeaks[b] > peaks[b]) {
                peaks[b] = chPeaks[b];
            }
        }
    }

    return peaks;
}

std::shared_ptr<WaveformData> WaveformBucketer::generateWaveformData(
    const core::MediaId& mediaId,
    const float* samples,
    size_t sampleCount,
    int32_t sampleRate,
    int32_t bucketSize
) {
    auto data = std::make_shared<WaveformData>();
    data->mediaId = mediaId;
    data->sampleRate = sampleRate;
    data->samplesPerBucket = bucketSize;

    if (!samples || sampleCount == 0 || bucketSize <= 0) {
        return data;
    }

    auto peaks = computePeakBuckets(samples, sampleCount, static_cast<size_t>(bucketSize));
    data->buckets.reserve(peaks.size());

    for (float p : peaks) {
        WaveformBucket b;
        b.minPeak = -p;
        b.maxPeak = p;
        data->buckets.push_back(b);
    }

    return data;
}

} // namespace catchim::media
