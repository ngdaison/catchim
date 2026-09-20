#include "Track.h"

namespace catchim::editor {

Track::Track(core::TrackId id, TrackType type, std::string name)
    : id_(std::move(id))
    , type_(type)
    , name_(std::move(name))
{
}

bool Track::acceptsClipType(ClipType clipType) const noexcept {
    switch (type_) {
        case TrackType::Video:
            return clipType == ClipType::Video || clipType == ClipType::Image;
        case TrackType::Audio:
            return clipType == ClipType::Audio;
        case TrackType::Text:
            return clipType == ClipType::Text;
        case TrackType::Graphic:
            return clipType == ClipType::Graphic || clipType == ClipType::Sticker;
        case TrackType::Effect:
            return clipType == ClipType::Effect;
    }
    return false;
}

const Clip* Track::findClip(const core::ClipId& clipId) const noexcept {
    for (const auto& clip : clips_) {
        if (clip.id() == clipId) return &clip;
    }
    return nullptr;
}

Clip* Track::findClip(const core::ClipId& clipId) noexcept {
    for (auto& clip : clips_) {
        if (clip.id() == clipId) return &clip;
    }
    return nullptr;
}

bool Track::canPlace(
    core::TimelineTime start,
    core::TimelineTime duration,
    const std::optional<core::ClipId>& excludeClipId
) const noexcept {
    core::TimelineTime end = start + duration;
    for (const auto& clip : clips_) {
        if (excludeClipId.has_value() && clip.id() == *excludeClipId) {
            continue;
        }
        // Collision check: start < clip.end && end > clip.start
        if (start < clip.endTime() && end > clip.startTime()) {
            return false;
        }
    }
    return true;
}

bool Track::insertClip(Clip clip) {
    if (!acceptsClipType(clip.type())) return false;
    if (!canPlace(clip.startTime(), clip.duration(), std::nullopt)) return false;

    clips_.push_back(std::move(clip));
    sortClips();
    return true;
}

std::optional<Clip> Track::removeClip(const core::ClipId& clipId) {
    auto it = std::find_if(clips_.begin(), clips_.end(), [&](const Clip& c) {
        return c.id() == clipId;
    });
    if (it != clips_.end()) {
        Clip removed = std::move(*it);
        clips_.erase(it);
        return removed;
    }
    return std::nullopt;
}

core::TimelineTime Track::maxEndTime() const noexcept {
    core::TimelineTime maxTime(0);
    for (const auto& clip : clips_) {
        maxTime = std::max(maxTime, clip.endTime());
    }
    return maxTime;
}

void Track::sortClips() {
    std::sort(clips_.begin(), clips_.end(), [](const Clip& a, const Clip& b) {
        return a.startTime() < b.startTime();
    });
}

} // namespace catchim::editor
