#include "TimelineManager.h"
#include "editor/scene/SceneHierarchyUtils.h"
#include <algorithm>

namespace catchim::editor {

TimelineManager::TimelineManager(Project& project)
    : project_(project) {}

Timeline* TimelineManager::activeTimeline() noexcept {
    return project_.activeTimeline();
}

const Timeline* TimelineManager::activeTimeline() const noexcept {
    return project_.activeTimeline();
}

core::TimelineTime TimelineManager::getTotalDuration() const noexcept {
    const auto* tl = activeTimeline();
    if (!tl) return core::TimelineTime(0);
    return SceneHierarchyUtils::calculateTotalDuration(*tl);
}

core::TimelineTime TimelineManager::getLastFrameTime(const core::FrameRate& fps) const noexcept {
    const auto total = getTotalDuration();
    if (total.ticks() <= 0) {
        return core::TimelineTime(0);
    }
    const auto frameDur = fps.frameDuration();
    if (total <= frameDur) {
        return core::TimelineTime(0);
    }
    return total - frameDur;
}

core::TrackId TimelineManager::addTrack(TrackType type, std::string name) {
    auto* tl = activeTimeline();
    if (!tl) return core::TrackId::generate();

    auto& track = tl->addTrack(type, std::move(name));
    project_.setDirty(true);
    notify();
    return track.id();
}

bool TimelineManager::removeTrack(const core::TrackId& trackId) {
    auto* tl = activeTimeline();
    if (!tl) return false;

    bool ok = tl->removeTrack(trackId);
    if (ok) {
        project_.setDirty(true);
        notify();
    }
    return ok;
}

bool TimelineManager::toggleTrackMute(const core::TrackId& trackId) {
    auto* tl = activeTimeline();
    if (!tl) return false;

    auto* track = tl->findTrack(trackId);
    if (!track) return false;

    track->setMuted(!track->isMuted());
    project_.setDirty(true);
    notify();
    return true;
}

bool TimelineManager::toggleTrackVisibility(const core::TrackId& trackId) {
    auto* tl = activeTimeline();
    if (!tl) return false;

    auto* track = tl->findTrack(trackId);
    if (!track) return false;

    track->setHidden(!track->isHidden());
    project_.setDirty(true);
    notify();
    return true;
}

bool TimelineManager::insertElement(const core::TrackId& trackId, Clip clip) {
    auto* tl = activeTimeline();
    if (!tl) return false;

    bool ok = tl->addClip(trackId, std::move(clip));
    if (ok) {
        project_.setDirty(true);
        notify();
    }
    return ok;
}

bool TimelineManager::deleteElements(const std::vector<core::ClipId>& clipIds) {
    auto* tl = activeTimeline();
    if (!tl || clipIds.empty()) return false;

    bool anyRemoved = false;
    for (const auto& id : clipIds) {
        if (tl->removeClip(id).has_value()) {
            anyRemoved = true;
        }
    }

    if (anyRemoved) {
        project_.setDirty(true);
        notify();
    }
    return anyRemoved;
}

std::vector<core::ClipId> TimelineManager::duplicateElements(const std::vector<core::ClipId>& clipIds) {
    auto* tl = activeTimeline();
    if (!tl || clipIds.empty()) return {};

    std::vector<core::ClipId> newIds;
    for (const auto& id : clipIds) {
        auto* track = tl->findTrackContainingClip(id);
        const auto* clip = tl->findClip(id);
        if (!track || !clip) continue;

        Clip dup = *clip;
        dup.setId(core::ClipId::generate());
        dup.setStartTime(clip->endTime()); // place after original
        newIds.push_back(dup.id());
        track->clips().push_back(std::move(dup));
    }

    if (!newIds.empty()) {
        project_.setDirty(true);
        notify();
    }
    return newIds;
}

bool TimelineManager::splitElements(const std::vector<core::ClipId>& clipIds, core::TimelineTime splitTime) {
    auto* tl = activeTimeline();
    if (!tl || clipIds.empty()) return false;

    bool anySplit = false;
    for (const auto& id : clipIds) {
        auto* track = tl->findTrackContainingClip(id);
        auto* clip = tl->findClip(id);
        if (!track || !clip) continue;

        if (splitTime <= clip->startTime() || splitTime >= clip->endTime()) {
            continue;
        }

        const auto originalEnd = clip->endTime();
        const auto firstDuration = splitTime - clip->startTime();
        const auto secondDuration = originalEnd - splitTime;

        Clip rightPart = *clip;
        rightPart.setId(core::ClipId::generate());
        rightPart.setStartTime(splitTime);
        rightPart.setDuration(secondDuration);

        clip->setDuration(firstDuration);
        track->clips().push_back(std::move(rightPart));
        anySplit = true;
    }

    if (anySplit) {
        project_.setDirty(true);
        notify();
    }
    return anySplit;
}

bool TimelineManager::moveElement(
    const core::ClipId& clipId,
    const core::TrackId& newTrackId,
    core::TimelineTime newStartTime
) {
    auto* tl = activeTimeline();
    if (!tl) return false;

    auto optClip = tl->removeClip(clipId);
    if (!optClip.has_value()) return false;

    optClip->setStartTime(newStartTime);
    bool ok = tl->addClip(newTrackId, std::move(*optClip));
    if (ok) {
        project_.setDirty(true);
        notify();
    }
    return ok;
}

void TimelineManager::subscribe(TimelineChangeListener listener) {
    listeners_.push_back(std::move(listener));
}

void TimelineManager::notify() {
    for (const auto& l : listeners_) {
        if (l) l();
    }
}

} // namespace catchim::editor
