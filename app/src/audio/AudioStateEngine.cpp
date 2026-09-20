#include "audio/AudioStateEngine.h"

namespace catchim::audio {

double AudioStateEngine::clampDb(double value) noexcept {
    if (!std::isfinite(value)) {
        return 0.0;
    }
    return std::clamp(value, kVolumeDbMin, kVolumeDbMax);
}

double AudioStateEngine::dBToLinear(double db) noexcept {
    return std::pow(10.0, clampDb(db) / 20.0);
}

double AudioStateEngine::linearToDb(double linear) noexcept {
    if (!std::isfinite(linear) || linear <= 1e-6) {
        return kVolumeDbMin;
    }
    return clampDb(20.0 * std::log10(linear));
}

double AudioStateEngine::getElementVolume(const editor::Clip& clip) noexcept {
    const auto& p = clip.params();
    if (p.contains("volume") && p["volume"].is_number()) {
        return p["volume"].get<double>();
    }
    return 0.0;
}

bool AudioStateEngine::isElementMuted(const editor::Clip& clip) noexcept {
    if (clip.isMuted()) {
        return true;
    }
    const auto& p = clip.params();
    if (p.contains("muted") && p["muted"].is_boolean()) {
        return p["muted"].get<bool>();
    }
    return false;
}

bool AudioStateEngine::hasAnimatedVolume(const editor::Clip& clip) noexcept {
    const auto* ch = clip.findAnimationChannel("volume");
    return ch != nullptr && !ch->empty();
}

double AudioStateEngine::resolveEffectiveAudioGain(
    const editor::Clip& clip,
    bool trackMuted,
    double localTimeSeconds
) noexcept {
    if (trackMuted || isElementMuted(clip)) {
        return 0.0;
    }

    const auto* ch = clip.findAnimationChannel("volume");
    if (!ch || ch->empty()) {
        return dBToLinear(getElementVolume(clip));
    }

    core::TimelineTime evalTime = core::TimelineTime::fromSeconds(std::max(0.0, localTimeSeconds));
    double resolvedDb = ch->getValueAt(evalTime);
    return dBToLinear(resolvedDb);
}

std::vector<double> AudioStateEngine::buildWaveformGainSamples(
    const editor::Clip& clip,
    int count
) {
    if (count <= 0) {
        return {};
    }

    if (!hasAnimatedVolume(clip)) {
        double gain = isElementMuted(clip) ? 0.0 : dBToLinear(getElementVolume(clip));
        return std::vector<double>(static_cast<size_t>(count), gain);
    }

    double durationSec = clip.duration().toSeconds();
    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i) {
        double localTime = ((static_cast<double>(i) + 0.5) / static_cast<double>(count)) * durationSec;
        samples.push_back(resolveEffectiveAudioGain(clip, false, localTime));
    }

    return samples;
}

std::vector<AudioAutomationPoint> AudioStateEngine::buildAudioGainAutomation(
    const editor::Clip& clip,
    bool trackMuted,
    double fromLocalTime,
    double toLocalTime,
    double stepSeconds
) {
    double startTime = std::max(0.0, fromLocalTime);
    double endTime = std::max(startTime, toLocalTime);

    if (!hasAnimatedVolume(clip)) {
        double gain = (trackMuted || isElementMuted(clip))
            ? 0.0
            : dBToLinear(getElementVolume(clip));
        return {
            {startTime, gain},
            {endTime, gain}
        };
    }

    double safeStep = (stepSeconds > 0.0 && std::isfinite(stepSeconds))
        ? stepSeconds
        : kDefaultGainStepSeconds;

    std::vector<AudioAutomationPoint> points;
    // Estimate capacity
    size_t countEstimate = static_cast<size_t>(std::ceil((endTime - startTime) / safeStep)) + 2;
    points.reserve(countEstimate);

    for (double t = startTime; t < endTime; t += safeStep) {
        points.push_back({t, resolveEffectiveAudioGain(clip, trackMuted, t)});
    }

    points.push_back({endTime, resolveEffectiveAudioGain(clip, trackMuted, endTime)});
    return points;
}

} // namespace catchim::audio
