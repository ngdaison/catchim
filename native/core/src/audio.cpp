#include "opencut/audio.hpp"

#include <algorithm>
#include <cmath>

namespace opencut {

namespace {

float smoothstep(float edge0, float edge1, float x) noexcept {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

} // namespace

float evaluate_volume_envelope(
    TimelineTick time,
    std::span<const AudioKeyframe> keyframes,
    float default_volume) noexcept {
    if (keyframes.empty()) {
        return default_volume;
    }

    if (time <= keyframes.front().time) {
        return keyframes.front().value;
    }

    if (time >= keyframes.back().time) {
        return keyframes.back().value;
    }

    // Binary search for keyframe segment
    auto it = std::upper_bound(keyframes.begin(), keyframes.end(), time,
        [](TimelineTick t, const AudioKeyframe& kf) {
            return t < kf.time;
        });

    if (it == keyframes.begin()) {
        return keyframes.front().value;
    }

    const auto& kf1 = *(it - 1);
    const auto& kf2 = *it;

    if (kf1.time == kf2.time) {
        return kf1.value;
    }

    float t = static_cast<float>(time - kf1.time) / static_cast<float>(kf2.time - kf1.time);

    switch (kf1.interp) {
        case KeyframeInterpolation::Hold:
            return kf1.value;
        case KeyframeInterpolation::EaseInOut: {
            float eased_t = t * t * (3.0f - 2.0f * t);
            return kf1.value + (kf2.value - kf1.value) * eased_t;
        }
        case KeyframeInterpolation::Linear:
        default:
            return kf1.value + (kf2.value - kf1.value) * t;
    }
}

float evaluate_fade_multiplier(
    TimelineTick clip_offset,
    TimelineTick clip_duration,
    const AudioFade& fade) noexcept {
    if (clip_offset < 0 || clip_offset > clip_duration) {
        return 0.0f;
    }

    float mult = 1.0f;

    // Fade in
    if (fade.fade_in_ticks > 0 && clip_offset < fade.fade_in_ticks) {
        float t = static_cast<float>(clip_offset) / static_cast<float>(fade.fade_in_ticks);
        // Exponential-like or smooth fade curve for audio
        mult *= std::sin(t * 1.57079632679f);
    }

    // Fade out
    TimelineTick fade_out_start = clip_duration - fade.fade_out_ticks;
    if (fade.fade_out_ticks > 0 && clip_offset > fade_out_start) {
        float t = static_cast<float>(clip_duration - clip_offset) / static_cast<float>(fade.fade_out_ticks);
        mult *= std::sin(std::max(t, 0.0f) * 1.57079632679f);
    }

    return std::clamp(mult, 0.0f, 1.0f);
}

std::vector<WaveformPeak> decimate_waveform_peaks(
    std::span<const float> samples,
    std::size_t num_bins) {
    if (samples.empty() || num_bins == 0) {
        return {};
    }

    std::vector<WaveformPeak> peaks(num_bins);
    double samples_per_bin = static_cast<double>(samples.size()) / static_cast<double>(num_bins);

    for (std::size_t bin = 0; bin < num_bins; ++bin) {
        std::size_t start_idx = static_cast<std::size_t>(bin * samples_per_bin);
        std::size_t end_idx = static_cast<std::size_t>((bin + 1) * samples_per_bin);
        if (end_idx > samples.size()) {
            end_idx = samples.size();
        }
        if (start_idx >= end_idx && start_idx < samples.size()) {
            end_idx = start_idx + 1;
        }

        float min_val = 0.0f;
        float max_val = 0.0f;
        if (start_idx < end_idx) {
            min_val = samples[start_idx];
            max_val = samples[start_idx];
            for (std::size_t i = start_idx + 1; i < end_idx; ++i) {
                float s = samples[i];
                if (s < min_val) min_val = s;
                if (s > max_val) max_val = s;
            }
        }
        peaks[bin] = WaveformPeak{.min_val = min_val, .max_val = max_val};
    }

    return peaks;
}

} // namespace opencut
