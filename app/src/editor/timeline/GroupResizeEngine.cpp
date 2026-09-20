#include "editor/timeline/GroupResizeEngine.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

namespace {

core::TimelineTime getSourceDeltaForClipDelta(const GroupResizeMember& member, core::TimelineTime clipDelta) noexcept {
    if (std::abs(member.retimeRate - 1.0) < 1e-6) {
        return clipDelta;
    }
    int64_t sourceTicks = static_cast<int64_t>(std::round(static_cast<double>(clipDelta.ticks()) * member.retimeRate));
    return core::TimelineTime::fromTicks(sourceTicks);
}

core::TimelineTime getVisibleSourceSpanForDuration(const GroupResizeMember& member, core::TimelineTime duration) noexcept {
    if (std::abs(member.retimeRate - 1.0) < 1e-6) {
        return duration;
    }
    int64_t sourceTicks = static_cast<int64_t>(std::round(static_cast<double>(duration.ticks()) * member.retimeRate));
    return core::TimelineTime::fromTicks(sourceTicks);
}

core::TimelineTime getDurationForVisibleSourceSpan(const GroupResizeMember& member, core::TimelineTime sourceSpan) noexcept {
    if (std::abs(member.retimeRate - 1.0) < 1e-6 || member.retimeRate <= 0.0) {
        return sourceSpan;
    }
    int64_t durTicks = static_cast<int64_t>(std::round(static_cast<double>(sourceSpan.ticks()) / member.retimeRate));
    return core::TimelineTime::fromTicks(durTicks);
}

core::TimelineTime getSourceDuration(const GroupResizeMember& member) noexcept {
    if (member.sourceDuration.has_value()) {
        return member.sourceDuration.value();
    }
    return member.trimStart + getVisibleSourceSpanForDuration(member, member.duration) + member.trimEnd;
}

core::TimelineTime getMinimumAllowedDeltaTime(
    const GroupResizeMember& member,
    ResizeSide side,
    core::TimelineTime minDuration
) noexcept {
    if (side == ResizeSide::Right) {
        return minDuration - member.duration;
    }

    core::TimelineTime leftNeighborFloor = member.leftNeighborBound.has_value()
        ? (member.leftNeighborBound.value() - member.startTime)
        : (core::TimelineTime(0) - member.startTime);

    if (!member.sourceDuration.has_value()) {
        return leftNeighborFloor;
    }

    core::TimelineTime maxSourceExtension = getDurationForVisibleSourceSpan(
        member,
        getVisibleSourceSpanForDuration(member, member.duration) + member.trimStart
    ) - member.duration;

    return std::max(leftNeighborFloor, core::TimelineTime(0) - maxSourceExtension);
}

std::optional<core::TimelineTime> getMaximumAllowedDeltaTime(
    const GroupResizeMember& member,
    ResizeSide side,
    core::TimelineTime minDuration
) noexcept {
    if (side == ResizeSide::Left) {
        return member.duration - minDuration;
    }

    std::optional<core::TimelineTime> rightNeighborCeiling = member.rightNeighborBound.has_value()
        ? std::optional<core::TimelineTime>(member.rightNeighborBound.value() - (member.startTime + member.duration))
        : std::nullopt;

    if (!member.sourceDuration.has_value()) {
        return rightNeighborCeiling;
    }

    core::TimelineTime maxVisibleSourceSpan = getSourceDuration(member) - member.trimStart;
    core::TimelineTime maxDuration = getDurationForVisibleSourceSpan(member, maxVisibleSourceSpan);
    core::TimelineTime sourceDurationCeiling = maxDuration - member.duration;

    if (!rightNeighborCeiling.has_value()) {
        return sourceDurationCeiling;
    }
    return std::min(rightNeighborCeiling.value(), sourceDurationCeiling);
}

} // anonymous namespace

GroupResizeResult GroupResizeEngine::computeGroupResize(
    const std::vector<GroupResizeMember>& members,
    ResizeSide side,
    core::TimelineTime deltaTime,
    const core::FrameRate& fps
) noexcept {
    if (members.empty()) {
        return {core::TimelineTime(0), {}};
    }

    int64_t frameTicks = static_cast<int64_t>(
        std::round((core::TICKS_PER_SECOND_F64 * fps.denominator) / fps.numerator)
    );
    if (frameTicks <= 0) frameTicks = 1;
    core::TimelineTime minDuration = core::TimelineTime::fromTicks(frameTicks);

    core::TimelineTime minimumDeltaTime = getMinimumAllowedDeltaTime(members[0], side, minDuration);
    std::optional<core::TimelineTime> maximumDeltaTime = getMaximumAllowedDeltaTime(members[0], side, minDuration);

    for (size_t i = 1; i < members.size(); ++i) {
        minimumDeltaTime = std::max(minimumDeltaTime, getMinimumAllowedDeltaTime(members[i], side, minDuration));
        auto memberMax = getMaximumAllowedDeltaTime(members[i], side, minDuration);
        if (memberMax.has_value()) {
            maximumDeltaTime = maximumDeltaTime.has_value()
                ? std::min(maximumDeltaTime.value(), memberMax.value())
                : memberMax;
        }
    }

    core::TimelineTime clampedDeltaTime = maximumDeltaTime.has_value()
        ? deltaTime.clamp(minimumDeltaTime, maximumDeltaTime.value())
        : std::max(minimumDeltaTime, deltaTime);

    int64_t snappedTicks = core::RationalFrameRateHelper::roundFrameTicks(clampedDeltaTime.ticks(), fps);
    core::TimelineTime snappedDeltaTime = core::TimelineTime::fromTicks(snappedTicks);

    core::TimelineTime finalDeltaTime = maximumDeltaTime.has_value()
        ? snappedDeltaTime.clamp(minimumDeltaTime, maximumDeltaTime.value())
        : std::max(minimumDeltaTime, snappedDeltaTime);

    std::vector<GroupResizeUpdate> updates;
    updates.reserve(members.size());

    for (const auto& member : members) {
        core::TimelineTime sourceDelta = getSourceDeltaForClipDelta(member, finalDeltaTime);
        GroupResizePatch patch;

        if (side == ResizeSide::Left) {
            patch.trimStart = std::max(core::TimelineTime(0), member.trimStart + sourceDelta);
            patch.trimEnd = member.trimEnd;
            patch.startTime = member.startTime + finalDeltaTime;
            patch.duration = member.duration - finalDeltaTime;
        } else {
            patch.trimStart = member.trimStart;
            patch.trimEnd = std::max(core::TimelineTime(0), member.trimEnd - sourceDelta);
            patch.startTime = member.startTime;
            patch.duration = member.duration + finalDeltaTime;
        }

        updates.push_back(GroupResizeUpdate{
            member.trackId,
            member.elementId,
            patch
        });
    }

    return GroupResizeResult{finalDeltaTime, std::move(updates)};
}

std::vector<GroupResizeMember> GroupResizeEngine::buildResizeMembers(
    const Timeline& timeline,
    const std::vector<core::ClipId>& clipIds
) {
    std::vector<GroupResizeMember> members;
    members.reserve(clipIds.size());

    for (const auto& cid : clipIds) {
        const Track* track = timeline.findTrackContainingClip(cid);
        if (!track) continue;

        const Clip* clip = track->findClip(cid);
        if (!clip) continue;

        GroupResizeMember m;
        m.trackId = track->id();
        m.elementId = clip->id();
        m.startTime = clip->startTime();
        m.duration = clip->duration();
        m.trimStart = clip->trimStart();
        m.trimEnd = clip->trimEnd();
        m.sourceDuration = clip->sourceDuration();
        m.retimeRate = clip->getParam<double>("speed", 1.0);

        // Find neighbor bounds within the same track
        for (const auto& otherClip : track->clips()) {
            if (otherClip.id() == clip->id()) continue;

            if (otherClip.endTime() <= clip->startTime()) {
                if (!m.leftNeighborBound.has_value() || otherClip.endTime() > m.leftNeighborBound.value()) {
                    m.leftNeighborBound = otherClip.endTime();
                }
            } else if (otherClip.startTime() >= clip->endTime()) {
                if (!m.rightNeighborBound.has_value() || otherClip.startTime() < m.rightNeighborBound.value()) {
                    m.rightNeighborBound = otherClip.startTime();
                }
            }
        }

        members.push_back(m);
    }

    return members;
}

// GroupResizeCommand implementation
GroupResizeCommand::GroupResizeCommand(
    Timeline& timeline,
    std::vector<GroupResizeUpdate> updates,
    std::string name
)
    : timeline_(timeline)
    , updates_(std::move(updates))
    , name_(std::move(name))
{
}

bool GroupResizeCommand::execute() {
    previousStates_.clear();
    previousStates_.reserve(updates_.size());

    for (const auto& u : updates_) {
        Clip* clip = timeline_.findClip(u.elementId);
        if (!clip) continue;

        previousStates_.push_back(PreviousClipState{
            u.trackId,
            u.elementId,
            clip->startTime(),
            clip->duration(),
            clip->trimStart(),
            clip->trimEnd()
        });

        clip->setStartTime(u.patch.startTime);
        clip->setDuration(u.patch.duration);
        clip->setTrimStart(u.patch.trimStart);
        clip->setTrimEnd(u.patch.trimEnd);
    }

    return true;
}

bool GroupResizeCommand::undo() {
    for (const auto& prev : previousStates_) {
        Clip* clip = timeline_.findClip(prev.elementId);
        if (!clip) continue;

        clip->setStartTime(prev.startTime);
        clip->setDuration(prev.duration);
        clip->setTrimStart(prev.trimStart);
        clip->setTrimEnd(prev.trimEnd);
    }
    return true;
}

} // namespace catchim::editor
