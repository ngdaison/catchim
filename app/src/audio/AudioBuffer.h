#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

namespace catchim::audio {

class AudioBuffer {
public:
    AudioBuffer(int32_t channels = 2, int32_t sampleRate = 44100)
        : channels_(channels), sampleRate_(sampleRate) {}

    int32_t channels() const noexcept { return channels_; }
    int32_t sampleRate() const noexcept { return sampleRate_; }

    size_t frameCount() const noexcept {
        return channels_ > 0 ? (samples_.size() / channels_) : 0;
    }

    const std::vector<float>& samples() const noexcept { return samples_; }
    std::vector<float>& samples() noexcept { return samples_; }

    void resize(size_t frameCount) {
        samples_.resize(frameCount * channels_, 0.0f);
    }

    void clear() {
        std::fill(samples_.begin(), samples_.end(), 0.0f);
    }

private:
    int32_t channels_{2};
    int32_t sampleRate_{44100};
    std::vector<float> samples_; // Interleaved float samples [-1.0f, 1.0f]
};

} // namespace catchim::audio
