#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <cmath>
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace catchim::media {

struct SampleBucket {
    size_t bucketStart{0};
    size_t bucketEnd{0};

    bool operator==(const SampleBucket& other) const = default;
};

struct SourceWaveformSummary {
    std::string sourceKey;
    int32_t sampleRate{44100};
    size_t totalSamples{0};
    size_t bucketSize{128};
    std::vector<float> amplitudes;
};

class WaveformSummaryEngine {
public:
    static constexpr double RMS_ANALYSIS_WINDOW_SECONDS = 0.02;
    static constexpr size_t DEFAULT_SOURCE_WAVEFORM_BUCKET_SIZE = 128;

    /**
     * @brief Generates cache key for media or library audio ("media:id" or "library:id").
     * Mirrors web/src/media/waveform-summary.ts buildWaveformSourceKey.
     */
    static std::string buildWaveformSourceKey(
        std::string_view kind,
        std::string_view id
    );

    /**
     * @brief Computes peak amplitudes across multiple audio channels for given sample buckets.
     * Mirrors web/src/media/waveform-summary.ts computePeakBuckets.
     */
    static std::vector<float> computePeakBuckets(
        const std::vector<const float*>& channelData,
        size_t totalSamples,
        const std::vector<SampleBucket>& buckets
    );

    /**
     * @brief Computes Root Mean Square (RMS) energy across audio channels with sliding analysis windows.
     * Mirrors web/src/media/waveform-summary.ts computeRmsBuckets.
     */
    static std::vector<float> computeRmsBuckets(
        const std::vector<const float*>& channelData,
        size_t totalSamples,
        int32_t sampleRate,
        const std::vector<SampleBucket>& buckets,
        double windowSeconds = RMS_ANALYSIS_WINDOW_SECONDS
    );

    /**
     * @brief Builds a full SourceWaveformSummary by chunking samples into fixed-size buckets.
     * Mirrors web/src/media/waveform-summary.ts buildSourceWaveformSummary.
     */
    static SourceWaveformSummary buildSourceWaveformSummary(
        std::string sourceKey,
        const std::vector<const float*>& channelData,
        size_t totalSamples,
        int32_t sampleRate = 44100,
        size_t bucketSize = DEFAULT_SOURCE_WAVEFORM_BUCKET_SIZE
    );

    /**
     * @brief Calculates sample buckets for timeline waveform display bars, taking into account
     * zoom pixelsPerSecond, clip duration, source start time, and retime rate.
     * Mirrors web/src/media/waveform-summary.ts buildWaveformSampleBuckets.
     */
    static std::vector<SampleBucket> buildWaveformSampleBuckets(
        double clipLeftPx,
        double clipRightPx,
        size_t barCount,
        double pixelsPerSecond,
        double clipDurationSec,
        double sourceStartSec,
        double retimeRate,
        int32_t sampleRate,
        size_t maxSampleExclusive,
        double barStepPx
    );

    /**
     * @brief Sub-samples a cached SourceWaveformSummary for requested sample buckets.
     * Mirrors web/src/media/waveform-summary.ts sampleSourceWaveformSummary.
     */
    static std::vector<float> sampleSourceWaveformSummary(
        const SourceWaveformSummary& summary,
        const std::vector<SampleBucket>& buckets
    );
};

} // namespace catchim::media
