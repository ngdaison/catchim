#pragma once

#include "opencut/timeline.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace opencut {

enum class KeyframeInterpolation {
    Linear,
    EaseInOut,
    Hold,
};

struct AudioKeyframe {
    TimelineTick time = 0;
    float value = 1.0f; // 1.0 = 100% volume (0 dB)
    KeyframeInterpolation interp = KeyframeInterpolation::Linear;
};

struct AudioFade {
    TimelineTick fade_in_ticks = 0;
    TimelineTick fade_out_ticks = 0;
};

struct WaveformPeak {
    float min_val = 0.0f;
    float max_val = 0.0f;
};

[[nodiscard]] float evaluate_volume_envelope(
    TimelineTick time,
    std::span<const AudioKeyframe> keyframes,
    float default_volume = 1.0f) noexcept;

[[nodiscard]] float evaluate_fade_multiplier(
    TimelineTick clip_offset,
    TimelineTick clip_duration,
    const AudioFade& fade) noexcept;

[[nodiscard]] std::vector<WaveformPeak> decimate_waveform_peaks(
    std::span<const float> samples,
    std::size_t num_bins);

} // namespace opencut
