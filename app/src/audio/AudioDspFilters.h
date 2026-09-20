#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace catchim::audio {

enum class BiquadFilterType {
    Lowpass,
    Highpass,
    PeakingEQ,
    LowShelf,
    HighShelf
};

class BiquadFilter {
public:
    BiquadFilter() noexcept;

    void configure(
        BiquadFilterType type,
        uint32_t sampleRate,
        double cutoffHz,
        double Q = 0.70710678,
        double gainDb = 0.0
    ) noexcept;

    void reset() noexcept;

    float processSample(float in) noexcept;
    void processBuffer(float* samples, size_t count) noexcept;

    double b0() const noexcept { return b0_; }
    double b1() const noexcept { return b1_; }
    double b2() const noexcept { return b2_; }
    double a1() const noexcept { return a1_; }
    double a2() const noexcept { return a2_; }

private:
    double b0_{1.0};
    double b1_{0.0};
    double b2_{0.0};
    double a1_{0.0};
    double a2_{0.0};

    // Filter state (Direct Form II Transposed)
    double d1_{0.0};
    double d2_{0.0};
};

class ThreeBandEqualizer {
public:
    ThreeBandEqualizer(uint32_t sampleRate = 44100) noexcept;

    void setSampleRate(uint32_t sampleRate) noexcept;

    void setLowGain(double gainDb) noexcept;
    void setMidGain(double gainDb) noexcept;
    void setHighGain(double gainDb) noexcept;

    double lowGain() const noexcept { return lowGainDb_; }
    double midGain() const noexcept { return midGainDb_; }
    double highGain() const noexcept { return highGainDb_; }

    void process(float* samples, size_t count) noexcept;
    void processStereo(float* left, float* right, size_t length) noexcept;

    void reset() noexcept;

private:
    void reconfigure() noexcept;

    uint32_t sampleRate_{44100};
    double lowGainDb_{0.0};
    double midGainDb_{0.0};
    double highGainDb_{0.0};

    // Mono / Left filters
    BiquadFilter lowFilter_;
    BiquadFilter midFilter_;
    BiquadFilter highFilter_;

    // Right channel filters for stereo
    BiquadFilter lowFilterR_;
    BiquadFilter midFilterR_;
    BiquadFilter highFilterR_;
};

} // namespace catchim::audio
