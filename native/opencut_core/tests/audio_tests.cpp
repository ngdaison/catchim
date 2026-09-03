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

int main() {
    test_audio_keyframe_envelope();
    test_waveform_decimation();
    std::cout << "All audio tests passed successfully!\n";
    return 0;
}
