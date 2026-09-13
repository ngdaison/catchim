#include "opencut/audio.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>

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

void apply_gain_ramp(float* samples, std::size_t num_samples, float start_gain, float end_gain) noexcept {
    if (!samples || num_samples == 0) return;

    if (num_samples == 1) {
        samples[0] *= start_gain;
        return;
    }

    float step = (end_gain - start_gain) / static_cast<float>(num_samples - 1);
    float current_gain = start_gain;

    for (std::size_t i = 0; i < num_samples; ++i) {
        samples[i] *= current_gain;
        current_gain += step;
    }
}

void mix_audio_buffers(float* dest, const float* src, std::size_t num_samples, float volume) noexcept {
    if (!dest || !src || num_samples == 0) return;

    for (std::size_t i = 0; i < num_samples; ++i) {
        float mixed = dest[i] + src[i] * volume;
        dest[i] = std::clamp(mixed, -1.0f, 1.0f);
    }
}

void resample_audio_linear(const float* src, std::size_t src_len, float* dst, std::size_t dst_len) noexcept {
    if (!src || !dst || src_len == 0 || dst_len == 0) return;

    if (src_len == 1 || dst_len == 1) {
        std::fill_n(dst, dst_len, src[0]);
        return;
    }

    double ratio = static_cast<double>(src_len - 1) / static_cast<double>(dst_len - 1);

    for (std::size_t i = 0; i < dst_len; ++i) {
        double src_pos = i * ratio;
        std::size_t idx0 = static_cast<std::size_t>(src_pos);
        std::size_t idx1 = std::min(idx0 + 1, src_len - 1);
        float frac = static_cast<float>(src_pos - idx0);

        dst[i] = src[idx0] * (1.0f - frac) + src[idx1] * frac;
    }
}

void compute_peak_buckets(
    std::span<const float* const> channels,
    std::span<const std::uint32_t> bucket_starts,
    std::span<const std::uint32_t> bucket_ends,
    float* out_peaks) noexcept {
    if (!out_peaks || bucket_starts.empty() || bucket_starts.size() != bucket_ends.size()) {
        return;
    }

    const std::size_t num_buckets = bucket_starts.size();
    for (std::size_t i = 0; i < num_buckets; ++i) {
        const std::uint32_t start = bucket_starts[i];
        const std::uint32_t end = bucket_ends[i];
        float peak = 0.0f;

        for (const float* ch : channels) {
            if (!ch) continue;
            for (std::uint32_t j = start; j < end; ++j) {
                float abs_val = std::abs(ch[j]);
                if (abs_val > peak) {
                    peak = abs_val;
                }
            }
        }
        out_peaks[i] = peak;
    }
}

void compute_rms_buckets(
    std::span<const float* const> channels,
    std::uint32_t max_window_length,
    std::span<const std::uint32_t> bucket_starts,
    std::span<const std::uint32_t> bucket_ends,
    float* out_rms) noexcept {
    if (!out_rms || bucket_starts.empty() || bucket_starts.size() != bucket_ends.size() || channels.empty()) {
        return;
    }

    const std::size_t num_buckets = bucket_starts.size();
    const double num_channels_d = static_cast<double>(channels.size());
    const std::uint32_t safe_max_win = std::max(1u, max_window_length);

    for (std::size_t i = 0; i < num_buckets; ++i) {
        const std::uint32_t bucket_start = bucket_starts[i];
        const std::uint32_t bucket_end = bucket_ends[i];
        if (bucket_end <= bucket_start) {
            out_rms[i] = 0.0f;
            continue;
        }

        const std::uint32_t bucket_length = bucket_end - bucket_start;
        const std::uint32_t window_length = std::max(1u, std::min(bucket_length, safe_max_win));
        float max_mean_square = 0.0f;

        for (std::uint32_t win_start = bucket_start; win_start < bucket_end; ) {
            const std::uint32_t win_end = std::min(win_start + window_length, bucket_end);
            const std::uint32_t n = win_end - win_start;
            if (n > 0) {
                double sum = 0.0;
                for (const float* ch : channels) {
                    if (!ch) continue;
                    for (std::uint32_t j = win_start; j < win_end; ++j) {
                        const float v = ch[j];
                        sum += static_cast<double>(v) * static_cast<double>(v);
                    }
                }
                const float mean_square = static_cast<float>(sum / (static_cast<double>(n) * num_channels_d));
                if (mean_square > max_mean_square) {
                    max_mean_square = mean_square;
                }
            }
            win_start = win_end;
        }

        out_rms[i] = std::sqrt(max_mean_square);
    }
}

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
    float gain) noexcept {
    if (!output_data || !source_data || sample_rate <= 0.0 || source_sample_rate <= 0.0 || gain == 0.0f) {
        return;
    }
    const double safe_rate = retime_rate <= 0.0 ? 1.0 : retime_rate;

    for (std::size_t i = 0; i < rendered_length; ++i) {
        const std::size_t output_index = output_start_sample + i;
        if (output_index >= output_length) break;

        const double clip_time = static_cast<double>(i) / sample_rate;
        const double source_time = trim_start + (clip_time * safe_rate);
        const double source_index = source_time * source_sample_rate;
        if (source_index < 0.0) continue;
        if (source_index >= static_cast<double>(source_length)) break;

        const std::size_t lower_index = static_cast<std::size_t>(source_index);
        const std::size_t upper_index = std::min(source_length - 1, lower_index + 1);
        const float fraction = static_cast<float>(source_index - static_cast<double>(lower_index));

        const float interpolated = source_data[lower_index] * (1.0f - fraction) + source_data[upper_index] * fraction;
        output_data[output_index] += interpolated * gain;
    }
}

float compute_buffer_peak(const float* samples, std::size_t num_samples) noexcept {
    if (!samples || num_samples == 0) return 0.0f;
    float peak = 0.0f;
    for (std::size_t i = 0; i < num_samples; ++i) {
        float mag = std::abs(samples[i]);
        if (mag > peak) {
            peak = mag;
        }
    }
    return peak;
}

void clamp_buffer_samples(float* samples, std::size_t num_samples, float max_peak) noexcept {
    if (!samples || num_samples == 0 || max_peak <= 0.0f) return;
    const float min_val = -max_peak;
    const float max_val = max_peak;
    for (std::size_t i = 0; i < num_samples; ++i) {
        samples[i] = std::clamp(samples[i], min_val, max_val);
    }
}

void downmix_stereo_to_mono(const float* left, const float* right, float* out, std::size_t num_samples) noexcept {
    if (!left || !right || !out || num_samples == 0) return;
    for (std::size_t i = 0; i < num_samples; ++i) {
        out[i] = (left[i] + right[i]) * 0.5f;
    }
}

void downmix_channels_to_mono(const float* const* channels, std::size_t num_channels, float* out, std::size_t num_samples) noexcept {
    if (!channels || num_channels == 0 || !out || num_samples == 0) return;

    if (num_channels == 1) {
        if (channels[0] != nullptr && channels[0] != out) {
            std::memcpy(out, channels[0], num_samples * sizeof(float));
        }
        return;
    }

    if (num_channels == 2 && channels[0] != nullptr && channels[1] != nullptr) {
        downmix_stereo_to_mono(channels[0], channels[1], out, num_samples);
        return;
    }

    const float inv_channels = 1.0f / static_cast<float>(num_channels);
    for (std::size_t i = 0; i < num_samples; ++i) {
        float sum = 0.0f;
        for (std::size_t c = 0; c < num_channels; ++c) {
            if (channels[c]) {
                sum += channels[c][i];
            }
        }
        out[i] = sum * inv_channels;
    }
}

} // namespace opencut
