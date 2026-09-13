#include "opencut/audio.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut;

void test_audio_keyframe_envelope() {
    std::vector<AudioKeyframe> kfs = {
        AudioKeyframe{.time = 0, .value = 0.0f, .interp = KeyframeInterpolation::Linear},
        AudioKeyframe{.time = 1000, .value = 1.0f, .interp = KeyframeInterpolation::Linear},
    };

    // Before first keyframe
    assert(evaluate_volume_envelope(-100, kfs) == 0.0f);
    // At first keyframe
    assert(evaluate_volume_envelope(0, kfs) == 0.0f);
    // Midpoint: 500 ticks -> 0.5
    assert(std::abs(evaluate_volume_envelope(500, kfs) - 0.5f) < 1e-4f);
    // At end keyframe
    assert(evaluate_volume_envelope(1000, kfs) == 1.0f);
    // After end keyframe
    assert(evaluate_volume_envelope(2000, kfs) == 1.0f);
}

void test_waveform_decimation() {
    std::vector<float> samples = {-0.5f, 0.8f, -0.2f, 0.4f, -0.9f, 0.1f};
    auto peaks = decimate_waveform_peaks(samples, 2);
    assert(peaks.size() == 2);
    // First bin (first 3 samples): min -0.5, max 0.8
    assert(peaks[0].min_val == -0.5f);
    assert(peaks[0].max_val == 0.8f);
    // Second bin (next 3 samples): min -0.9, max 0.4
    assert(peaks[1].min_val == -0.9f);
    assert(peaks[1].max_val == 0.4f);
}

void test_waveform_buckets() {
    std::vector<float> ch1 = {0.1f, -0.8f, 0.4f, 0.3f, -0.6f, 0.9f};
    const float* channels[] = {ch1.data()};
    std::vector<uint32_t> starts = {0, 3};
    std::vector<uint32_t> ends = {3, 6};
    std::vector<float> peaks(2, 0.0f);

    compute_peak_buckets(
        std::span<const float* const>(channels, 1),
        starts,
        ends,
        peaks.data()
    );

    assert(std::abs(peaks[0] - 0.8f) < 1e-4f);
    assert(std::abs(peaks[1] - 0.9f) < 1e-4f);

    std::vector<float> rms(2, 0.0f);
    compute_rms_buckets(
        std::span<const float* const>(channels, 1),
        3,
        starts,
        ends,
        rms.data()
    );
    assert(rms[0] > 0.0f);
    assert(rms[1] > 0.0f);
}

void test_audio_mix_retime() {
    std::vector<float> src = {1.0f, 1.0f, 1.0f, 1.0f};
    std::vector<float> dst(4, 0.0f);

    mix_audio_channel_retime(
        dst.data(),
        0,
        4,
        4,
        1.0,
        src.data(),
        src.size(),
        1.0,
        0.0,
        1.0,
        0.5f
    );

    for (float v : dst) {
        assert(std::abs(v - 0.5f) < 1e-4f);
    }
}

void test_peak_and_clamp() {
    std::vector<float> samples = {0.1f, -0.95f, 0.4f, 1.2f, -1.5f};
    float peak = compute_buffer_peak(samples.data(), samples.size());
    assert(std::abs(peak - 1.5f) < 1e-4f);

    clamp_buffer_samples(samples.data(), samples.size(), 0.98f);
    float clamped_peak = compute_buffer_peak(samples.data(), samples.size());
    assert(clamped_peak <= 0.980001f);
}

void test_downmix() {
    std::vector<float> left = {1.0f, 0.5f, -0.2f, 0.8f};
    std::vector<float> right = {0.0f, 0.5f, 0.2f, -0.4f};
    std::vector<float> out(4);

    downmix_stereo_to_mono(left.data(), right.data(), out.data(), 4);
    assert(std::abs(out[0] - 0.5f) < 1e-4f);
    assert(std::abs(out[1] - 0.5f) < 1e-4f);
    assert(std::abs(out[2] - 0.0f) < 1e-4f);
    assert(std::abs(out[3] - 0.2f) < 1e-4f);

    const float* channels[] = {left.data(), right.data()};
    std::vector<float> out2(4);
    downmix_channels_to_mono(channels, 2, out2.data(), 4);
    assert(std::abs(out2[0] - 0.5f) < 1e-4f);
    assert(std::abs(out2[3] - 0.2f) < 1e-4f);
}

int main() {
    test_audio_keyframe_envelope();
    test_waveform_decimation();
    test_waveform_buckets();
    test_audio_mix_retime();
    test_peak_and_clamp();
    test_downmix();
    std::cout << "All audio tests passed successfully!\n";
    return 0;
}
