#include "editor/timeline/GroupMoveEngine.h"
#include <algorithm>
#include <unordered_set>

namespace catchim::editor {

std::optional<MoveGroup> GroupMoveEngine::buildMoveGroup(
    const Timeline& timeline,
    const core::TrackId& anchorTrackId,
    const core::ClipId& anchorClipId,
    const std::vector<std::pair<core::TrackId, core::ClipId>>& selectedElements
) {
    const auto* anchorTrack = timeline.findTrack(anchorTrackId);
    if (!anchorTrack) return std::nullopt;

    const auto* anchorClip = anchorTrack->findClip(anchorClipId);
    if (!anchorClip) return std::nullopt;

    GroupMember anchorMember{
        anchorTrackId,
        anchorClipId,
        anchorClip->type(),
        anchorClip->startTime(),
        anchorClip->duration(),
        core::TimelineTime(0)
    };

    std::vector<GroupMember> members;
    std::unordered_set<std::string> seen;

    auto addMember = [&](const core::TrackId& trkId, const core::ClipId& clpId) {
        if (seen.find(clpId.str()) != seen.end()) return;
        seen.insert(clpId.str());

        const auto* trk = timeline.findTrack(trkId);
        if (!trk) return;
        const auto* clp = trk->findClip(clpId);
        if (!clp) return;

        core::TimelineTime offset = clp->startTime() - anchorClip->startTime();
        members.push_back({
            trkId,
            clpId,
            clp->type(),
            clp->startTime(),
            clp->duration(),
            offset
        });
    };

    // Always include anchor
    addMember(anchorTrackId, anchorClipId);

    // Include other selected clips
    for (const auto& [trkId, clpId] : selectedElements) {
        addMember(trkId, clpId);
    }

    if (members.empty()) return std::nullopt;

    return MoveGroup{anchorMember, std::move(members)};
}

GroupMovePlan GroupMoveEngine::resolveGroupMove(
    const MoveGroup& group,
    core::TimelineTime proposedAnchorTime
) {
    // Find the minimum time offset among all group members
    core::TimelineTime minOffset = core::TimelineTime(0);
    for (const auto& m : group.members) {
        if (m.timeOffset < minOffset) {
            minOffset = m.timeOffset;
        }
    }

    // Clamp proposed anchor time so no member starts before t=0
    core::TimelineTime clampedAnchor = proposedAnchorTime;
    if (clampedAnchor + minOffset < core::TimelineTime(0)) {
        clampedAnchor = core::TimelineTime::fromTicks(-minOffset.ticks());
    }

    GroupMovePlan plan;
    plan.clampedAnchorTime = clampedAnchor;

    for (const auto& m : group.members) {
        core::TimelineTime newTime = clampedAnchor + m.timeOffset;
        plan.moves.push_back({
            m.trackId,
            m.trackId,
            m.clipId,
            newTime
        });
    }

    return plan;
}

GroupMoveCommand::GroupMoveCommand(Timeline& timeline, GroupMovePlan plan)
    : timeline_(timeline)
    , plan_(std::move(plan))
{
}

bool GroupMoveCommand::execute() {
    originalStates_.clear();
    std::vector<Clip> extracted;
    extracted.reserve(plan_.moves.size());

    for (const auto& move : plan_.moves) {
        const auto* clip = timeline_.findClip(move.clipId);
        if (!clip) return false;
        originalStates_.push_back({move.sourceTrackId, move.clipId, clip->startTime()});
    }

    for (const auto& move : plan_.moves) {
        auto removed = timeline_.removeClip(move.clipId);
        if (!removed) return false;
        extracted.push_back(std::move(*removed));
    }

    for (size_t i = 0; i < plan_.moves.size(); ++i) {
        const auto& move = plan_.moves[i];
        extracted[i].setStartTime(move.newStartTime);
        timeline_.addClip(move.targetTrackId, std::move(extracted[i]));
    }
    return true;
}

bool GroupMoveCommand::undo() {
    std::vector<Clip> extracted;
    extracted.reserve(originalStates_.size());

    for (const auto& orig : originalStates_) {
        auto removed = timeline_.removeClip(orig.clipId);
        if (!removed) return false;
        extracted.push_back(std::move(*removed));
    }

    for (size_t i = 0; i < originalStates_.size(); ++i) {
        const auto& orig = originalStates_[i];
        extracted[i].setStartTime(orig.startTime);
        timeline_.addClip(orig.trackId, std::move(extracted[i]));
    }
    return true;
}

std::string GroupMoveCommand::name() const {
    return "Move Group of " + std::to_string(plan_.moves.size()) + " clips";
}

} // namespace catchim::editor
