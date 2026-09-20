#include "AudioWaveformBarEngine.h"
#include "AudioDisplayMetrics.h"
#include <cmath>
#include <algorithm>

namespace catchim::audio {

double AudioWaveformBarEngine::sampleGainAtClipTime(
    const std::vector<double>& samples,
    double clipTimeSec,
    double clipDurationSec
) noexcept {
    if (samples.empty() || clipDurationSec <= 0.0) {
        return 1.0;
    }

    double progress = std::clamp(clipTimeSec / clipDurationSec, 0.0, 1.0);
    double rawIndex = progress * static_cast<double>(samples.size() - 1);
    size_t lo = static_cast<size_t>(std::floor(rawIndex));
    size_t hi = std::min(samples.size() - 1, lo + 1);
    double frac = rawIndex - static_cast<double>(lo);

    return samples[lo] + (samples[hi] - samples[lo]) * frac;
}

std::vector<WaveformBar> AudioWaveformBarEngine::calculateWaveformBars(
    const std::vector<float>& amplitudes,
    double width,
    double height,
    const std::vector<double>& gainSamples,
    double clipDurationSec
) {
    std::vector<WaveformBar> bars;
    if (amplitudes.empty() || width <= 0.0 || height <= 0.0) {
        return bars;
    }

    size_t numBars = static_cast<size_t>(std::floor(width / BAR_STEP));
    bars.reserve(numBars);

    for (size_t i = 0; i < numBars; ++i) {
        double x = static_cast<double>(i) * BAR_STEP;
        double progress = (width > 0.0) ? (x / width) : 0.0;
        double clipTimeSec = progress * clipDurationSec;

        double gain = sampleGainAtClipTime(gainSamples, clipTimeSec, clipDurationSec);

        double rawIdx = progress * static_cast<double>(amplitudes.size() - 1);
        size_t lo = static_cast<size_t>(std::floor(rawIdx));
        size_t hi = std::min(amplitudes.size() - 1, lo + 1);
        double frac = rawIdx - static_cast<double>(lo);
        double rawAmp = static_cast<double>(amplitudes[lo]) + (static_cast<double>(amplitudes[hi]) - static_cast<double>(amplitudes[lo])) * frac;

        double effectiveAmp = rawAmp * gain;
        double barFraction = AudioDisplayMetrics::getBarFractionFromOutputAmplitude(effectiveAmp);
        double barH = std::max(1.0, barFraction * height);
        double barTop = (height - barH) * 0.5;
        bool isBurnt = (effectiveAmp > 1.0);

        bars.push_back(WaveformBar{
            .x = x,
            .top = barTop,
            .height = barH,
            .isBurnt = isBurnt
        });
    }

    return bars;
}

} // namespace catchim::audio
