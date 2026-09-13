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

void apply_gain_ramp(float* samples, std::size_t num_samples, float start_gain, float end_gain) noexcept;

void mix_audio_buffers(float* dest, const float* src, std::size_t num_samples, float volume) noexcept;

void resample_audio_linear(const float* src, std::size_t src_len, float* dst, std::size_t dst_len) noexcept;

void compute_peak_buckets(
    std::span<const float* const> channels,
    std::span<const std::uint32_t> bucket_starts,
    std::span<const std::uint32_t> bucket_ends,
    float* out_peaks) noexcept;

void compute_rms_buckets(
    std::span<const float* const> channels,
    std::uint32_t max_window_length,
    std::span<const std::uint32_t> bucket_starts,
    std::span<const std::uint32_t> bucket_ends,
    float* out_rms) noexcept;

void mix_audio_channel_retime(
    float* output_data,
    std::size_t output_start_sample,
    std::size_t rendered_length,
    std::size_t output_length,
    double sample_rate,
    const float* source_data,
    std::size_t source_length,
    double source_sample_rate,
    double trim_start,
    double retime_rate,
    float gain) noexcept;

[[nodiscard]] float compute_buffer_peak(const float* samples, std::size_t num_samples) noexcept;

void clamp_buffer_samples(float* samples, std::size_t num_samples, float max_peak) noexcept;

void downmix_stereo_to_mono(const float* left, const float* right, float* out, std::size_t num_samples) noexcept;

void downmix_channels_to_mono(const float* const* channels, std::size_t num_channels, float* out, std::size_t num_samples) noexcept;

} // namespace opencut
