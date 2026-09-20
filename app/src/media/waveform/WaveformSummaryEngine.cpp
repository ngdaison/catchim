#include "WaveformSummaryEngine.h"

namespace catchim::media {

std::string WaveformSummaryEngine::buildWaveformSourceKey(
    std::string_view kind,
    std::string_view id
) {
    std::string result;
    result.reserve(kind.size() + 1 + id.size());
    result.append(kind);
    result.push_back(':');
    result.append(id);
    return result;
}

std::vector<float> WaveformSummaryEngine::computePeakBuckets(
    const std::vector<const float*>& channelData,
    size_t totalSamples,
    const std::vector<SampleBucket>& buckets
) {
    std::vector<float> peaks(buckets.size(), 0.0f);
    if (channelData.empty() || totalSamples == 0) {
        return peaks;
    }

    for (size_t b = 0; b < buckets.size(); ++b) {
        const auto& bucket = buckets[b];
        size_t start = std::min(bucket.bucketStart, totalSamples);
        size_t end = std::min(bucket.bucketEnd, totalSamples);

        float maxPeak = 0.0f;
        for (const float* ch : channelData) {
            if (!ch) continue;
            for (size_t i = start; i < end; ++i) {
                float absVal = std::abs(ch[i]);
                if (absVal > maxPeak) {
                    maxPeak = absVal;
                }
            }
        }
        peaks[b] = maxPeak;
    }

    return peaks;
}

std::vector<float> WaveformSummaryEngine::computeRmsBuckets(
    const std::vector<const float*>& channelData,
    size_t totalSamples,
    int32_t sampleRate,
    const std::vector<SampleBucket>& buckets,
    double windowSeconds
) {
    std::vector<float> results(buckets.size(), 0.0f);
    if (channelData.empty() || totalSamples == 0 || sampleRate <= 0) {
        return results;
    }

    const size_t maxWindowLength = std::max<size_t>(1, static_cast<size_t>(sampleRate * windowSeconds));
    const size_t numChannels = channelData.size();

    for (size_t b = 0; b < buckets.size(); ++b) {
        const auto& bucket = buckets[b];
        size_t start = std::min(bucket.bucketStart, totalSamples);
        size_t end = std::min(bucket.bucketEnd, totalSamples);
        if (end <= start) {
            results[b] = 0.0f;
            continue;
        }

        size_t bucketLength = end - start;
        size_t windowLength = std::max<size_t>(1, std::min(bucketLength, maxWindowLength));
        double maxMeanSquare = 0.0;

        for (size_t winStart = start; winStart < end; ) {
            size_t winEnd = std::min(winStart + windowLength, end);
            size_t n = winEnd - winStart;
            if (n > 0) {
                double sum = 0.0;
                for (const float* ch : channelData) {
                    if (!ch) continue;
                    for (size_t i = winStart; i < winEnd; ++i) {
                        double v = static_cast<double>(ch[i]);
                        sum += v * v;
                    }
                }
                double meanSquare = sum / static_cast<double>(n * numChannels);
                if (meanSquare > maxMeanSquare) {
                    maxMeanSquare = meanSquare;
                }
            }
            winStart = winEnd;
        }

        results[b] = static_cast<float>(std::sqrt(maxMeanSquare));
    }

    return results;
}

SourceWaveformSummary WaveformSummaryEngine::buildSourceWaveformSummary(
    std::string sourceKey,
    const std::vector<const float*>& channelData,
    size_t totalSamples,
    int32_t sampleRate,
    size_t bucketSize
) {
    size_t safeBucketSize = std::max<size_t>(1, bucketSize);
    size_t bucketCount = (totalSamples + safeBucketSize - 1) / safeBucketSize;
    if (bucketCount == 0) {
        bucketCount = 1;
    }

    std::vector<SampleBucket> buckets(bucketCount);
    for (size_t i = 0; i < bucketCount; ++i) {
        buckets[i].bucketStart = i * safeBucketSize;
        buckets[i].bucketEnd = std::min(totalSamples, buckets[i].bucketStart + safeBucketSize);
    }

    std::vector<float> amplitudes = computePeakBuckets(channelData, totalSamples, buckets);

    return SourceWaveformSummary{
        std::move(sourceKey),
        sampleRate,
        totalSamples,
        safeBucketSize,
        std::move(amplitudes)
    };
}

std::vector<SampleBucket> WaveformSummaryEngine::buildWaveformSampleBuckets(
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
) {
    std::vector<SampleBucket> buckets(barCount);
    if (pixelsPerSecond <= 0.0 || sampleRate <= 0) {
        return buckets;
    }

    double rate = (retimeRate > 0.0) ? retimeRate : 1.0;

    for (size_t i = 0; i < barCount; ++i) {
        double bucketLeftPx = clipLeftPx + static_cast<double>(i) * barStepPx;
        double bucketRightPx = std::min(clipRightPx, bucketLeftPx + barStepPx);

        double clipStartSec = std::max(0.0, std::min(clipDurationSec, bucketLeftPx / pixelsPerSecond));
        double clipEndSec = std::max(clipStartSec, std::min(clipDurationSec, bucketRightPx / pixelsPerSecond));

        double sourceBucketStartSec = sourceStartSec + (clipStartSec * rate);
        double sourceBucketEndSec = sourceStartSec + (clipEndSec * rate);

        size_t bStart = static_cast<size_t>(std::max(0.0, std::floor(sourceBucketStartSec * sampleRate)));
        size_t bEnd = std::min(
            maxSampleExclusive,
            static_cast<size_t>(std::max(0.0, std::ceil(sourceBucketEndSec * sampleRate)))
        );

        buckets[i] = SampleBucket{bStart, bEnd};
    }

    return buckets;
}

std::vector<float> WaveformSummaryEngine::sampleSourceWaveformSummary(
    const SourceWaveformSummary& summary,
    const std::vector<SampleBucket>& buckets
) {
    std::vector<float> result(buckets.size(), 0.0f);
    if (summary.amplitudes.empty() || summary.bucketSize == 0) {
        return result;
    }

    for (size_t b = 0; b < buckets.size(); ++b) {
        const auto& bucket = buckets[b];
        if (bucket.bucketEnd <= bucket.bucketStart) {
            result[b] = 0.0f;
            continue;
        }

        size_t startIndex = bucket.bucketStart / summary.bucketSize;
        size_t endIndex = (bucket.bucketEnd + summary.bucketSize - 1) / summary.bucketSize;
        endIndex = std::min(summary.amplitudes.size(), std::max(startIndex + 1, endIndex));

        float maxAmp = 0.0f;
        for (size_t i = startIndex; i < endIndex; ++i) {
            if (summary.amplitudes[i] > maxAmp) {
                maxAmp = summary.amplitudes[i];
            }
        }
        result[b] = maxAmp;
    }

    return result;
}

} // namespace catchim::media
