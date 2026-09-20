#include "AudioMediaUtils.h"

#include <cmath>

namespace catchim::audio {

void AudioMediaUtils::downmixStereo(
    const float* left,
    const float* right,
    float* out,
    size_t length) noexcept {

    if (!left || !right || !out || length == 0) {
        return;
    }

    for (size_t i = 0; i < length; ++i) {
        out[i] = (left[i] + right[i]) * 0.5f;
    }
}

bool AudioMediaUtils::timelineHasAudio(const editor::Timeline& timeline) noexcept {
    for (const auto& track : timeline.audioTracks()) {
        if (!track.clips().empty()) {
            return true;
        }
    }

    for (const auto* track : timeline.allTracks()) {
        for (const auto& clip : track->clips()) {
            if (clip.type() == editor::ClipType::Audio) {
                return true;
            }
        }
    }

    return false;
}

std::vector<core::ClipId> AudioMediaUtils::collectAudibleClips(const editor::Timeline& timeline) {
    std::vector<core::ClipId> audibleIds;

    for (const auto* track : timeline.allTracks()) {
        for (const auto& clip : track->clips()) {
            if (clip.type() == editor::ClipType::Audio || clip.type() == editor::ClipType::Video) {
                audibleIds.push_back(clip.id());
            }
        }
    }

    return audibleIds;
}

double AudioMediaUtils::dBToLinear(double db) noexcept {
    if (db <= -100.0) return 0.0;
    return std::pow(10.0, db / 20.0);
}

double AudioMediaUtils::linearToDb(double linear) noexcept {
    if (linear <= 1e-5) return -100.0;
    return 20.0 * std::log10(linear);
}

} // namespace catchim::audio
