#include "audio/AudioDspFilters.h"
#include <cmath>
#include <algorithm>

namespace catchim::audio {

namespace {
constexpr double PI = 3.14159265358979323846;
}

BiquadFilter::BiquadFilter() noexcept {
    reset();
}

void BiquadFilter::reset() noexcept {
    d1_ = 0.0;
    d2_ = 0.0;
}

void BiquadFilter::configure(
    BiquadFilterType type,
    uint32_t sampleRate,
    double cutoffHz,
    double Q,
    double gainDb
) noexcept {
    if (sampleRate == 0) sampleRate = 44100;
    double sr = static_cast<double>(sampleRate);

    // Clamp cutoff between 10 Hz and Nyquist (0.49 * sr)
    cutoffHz = std::clamp(cutoffHz, 10.0, 0.49 * sr);
    if (Q <= 0.001) Q = 0.001;

    double omega0 = 2.0 * PI * cutoffHz / sr;
    double sinW = std::sin(omega0);
    double cosW = std::cos(omega0);
    double alpha = sinW / (2.0 * Q);

    double b0 = 1.0, b1 = 0.0, b2 = 0.0;
    double a0 = 1.0, a1 = 0.0, a2 = 0.0;

    switch (type) {
        case BiquadFilterType::Lowpass: {
            b0 = (1.0 - cosW) * 0.5;
            b1 = 1.0 - cosW;
            b2 = (1.0 - cosW) * 0.5;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosW;
            a2 = 1.0 - alpha;
            break;
        }
        case BiquadFilterType::Highpass: {
            b0 = (1.0 + cosW) * 0.5;
            b1 = -(1.0 + cosW);
            b2 = (1.0 + cosW) * 0.5;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosW;
            a2 = 1.0 - alpha;
            break;
        }
        case BiquadFilterType::PeakingEQ: {
            double A = std::pow(10.0, gainDb / 40.0);
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cosW;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cosW;
            a2 = 1.0 - alpha / A;
            break;
        }
        case BiquadFilterType::LowShelf: {
            double A = std::pow(10.0, gainDb / 40.0);
            double shelfAlpha = (sinW / 2.0) * std::sqrt(2.0);
            double temp = 2.0 * std::sqrt(A) * shelfAlpha;

            b0 = A * ((A + 1.0) - (A - 1.0) * cosW + temp);
            b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW);
            b2 = A * ((A + 1.0) - (A - 1.0) * cosW - temp);
            a0 = (A + 1.0) + (A - 1.0) * cosW + temp;
            a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW);
            a2 = (A + 1.0) + (A - 1.0) * cosW - temp;
            break;
        }
        case BiquadFilterType::HighShelf: {
            double A = std::pow(10.0, gainDb / 40.0);
            double shelfAlpha = (sinW / 2.0) * std::sqrt(2.0);
            double temp = 2.0 * std::sqrt(A) * shelfAlpha;

            b0 = A * ((A + 1.0) + (A - 1.0) * cosW + temp);
            b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW);
            b2 = A * ((A + 1.0) + (A - 1.0) * cosW - temp);
            a0 = (A + 1.0) - (A - 1.0) * cosW + temp;
            a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW);
            a2 = (A + 1.0) - (A - 1.0) * cosW - temp;
            break;
        }
    }

    if (std::abs(a0) > 1e-12) {
        b0_ = b0 / a0;
        b1_ = b1 / a0;
        b2_ = b2 / a0;
        a1_ = a1 / a0;
        a2_ = a2 / a0;
    } else {
        b0_ = 1.0;
        b1_ = 0.0;
        b2_ = 0.0;
        a1_ = 0.0;
        a2_ = 0.0;
    }
}

float BiquadFilter::processSample(float in) noexcept {
    double x = static_cast<double>(in);
    double y = b0_ * x + d1_;
    d1_ = b1_ * x - a1_ * y + d2_;
    d2_ = b2_ * x - a2_ * y;
    return static_cast<float>(y);
}

void BiquadFilter::processBuffer(float* samples, size_t count) noexcept {
    if (!samples || count == 0) return;
    for (size_t i = 0; i < count; ++i) {
        samples[i] = processSample(samples[i]);
    }
}

// ThreeBandEqualizer implementation
ThreeBandEqualizer::ThreeBandEqualizer(uint32_t sampleRate) noexcept
    : sampleRate_(sampleRate)
{
    reconfigure();
}

void ThreeBandEqualizer::setSampleRate(uint32_t sampleRate) noexcept {
    if (sampleRate == sampleRate_) return;
    sampleRate_ = sampleRate;
    reconfigure();
}

void ThreeBandEqualizer::setLowGain(double gainDb) noexcept {
    lowGainDb_ = gainDb;
    lowFilter_.configure(BiquadFilterType::LowShelf, sampleRate_, 120.0, 0.7071, lowGainDb_);
    lowFilterR_.configure(BiquadFilterType::LowShelf, sampleRate_, 120.0, 0.7071, lowGainDb_);
}

void ThreeBandEqualizer::setMidGain(double gainDb) noexcept {
    midGainDb_ = gainDb;
    midFilter_.configure(BiquadFilterType::PeakingEQ, sampleRate_, 1000.0, 1.0, midGainDb_);
    midFilterR_.configure(BiquadFilterType::PeakingEQ, sampleRate_, 1000.0, 1.0, midGainDb_);
}

void ThreeBandEqualizer::setHighGain(double gainDb) noexcept {
    highGainDb_ = gainDb;
    highFilter_.configure(BiquadFilterType::HighShelf, sampleRate_, 8000.0, 0.7071, highGainDb_);
    highFilterR_.configure(BiquadFilterType::HighShelf, sampleRate_, 8000.0, 0.7071, highGainDb_);
}

void ThreeBandEqualizer::reconfigure() noexcept {
    lowFilter_.configure(BiquadFilterType::LowShelf, sampleRate_, 120.0, 0.7071, lowGainDb_);
    midFilter_.configure(BiquadFilterType::PeakingEQ, sampleRate_, 1000.0, 1.0, midGainDb_);
    highFilter_.configure(BiquadFilterType::HighShelf, sampleRate_, 8000.0, 0.7071, highGainDb_);

    lowFilterR_.configure(BiquadFilterType::LowShelf, sampleRate_, 120.0, 0.7071, lowGainDb_);
    midFilterR_.configure(BiquadFilterType::PeakingEQ, sampleRate_, 1000.0, 1.0, midGainDb_);
    highFilterR_.configure(BiquadFilterType::HighShelf, sampleRate_, 8000.0, 0.7071, highGainDb_);
}

void ThreeBandEqualizer::reset() noexcept {
    lowFilter_.reset();
    midFilter_.reset();
    highFilter_.reset();
    lowFilterR_.reset();
    midFilterR_.reset();
    highFilterR_.reset();
}

void ThreeBandEqualizer::process(float* samples, size_t count) noexcept {
    if (!samples || count == 0) return;
    lowFilter_.processBuffer(samples, count);
    midFilter_.processBuffer(samples, count);
    highFilter_.processBuffer(samples, count);
}

void ThreeBandEqualizer::processStereo(float* left, float* right, size_t length) noexcept {
    if (left && length > 0) {
        lowFilter_.processBuffer(left, length);
        midFilter_.processBuffer(left, length);
        highFilter_.processBuffer(left, length);
    }
    if (right && length > 0) {
        lowFilterR_.processBuffer(right, length);
        midFilterR_.processBuffer(right, length);
        highFilterR_.processBuffer(right, length);
    }
}

} // namespace catchim::audio
