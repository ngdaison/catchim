#include "editor/timeline/GroupMoveSnapEngine.h"
#include "editor/timeline/TimelineSnapPointSources.h"
#include "editor/timeline/PlacementEngine.h"
#include <algorithm>

namespace catchim::editor {

std::vector<TrackPlacement> GroupMoveSnapEngine::getDisplayTrackPlacements(const Timeline& timeline) {
    std::vector<TrackPlacement> placements;
    const auto& overlays = timeline.overlayTracks();
    const auto& main = timeline.mainTrack();
    const auto& audios = timeline.audioTracks();

    placements.reserve(overlays.size() + 1 + audios.size());

    // Overlays
    for (size_t i = 0; i < overlays.size(); ++i) {
        placements.push_back(TrackPlacement{
            .trackId = overlays[i].id(),
            .trackType = overlays[i].type(),
            .section = GroupTrackSection::Overlay,
            .sectionIndex = static_cast<int>(i),
            .displayIndex = static_cast<int>(i)
        });
    }

    // Main
    const int mainDisplayIndex = static_cast<int>(overlays.size());
    placements.push_back(TrackPlacement{
        .trackId = main.id(),
        .trackType = main.type(),
        .section = GroupTrackSection::Main,
        .sectionIndex = -1,
        .displayIndex = mainDisplayIndex
    });

    // Audio
    for (size_t i = 0; i < audios.size(); ++i) {
        placements.push_back(TrackPlacement{
            .trackId = audios[i].id(),
            .trackType = audios[i].type(),
            .section = GroupTrackSection::Audio,
            .sectionIndex = static_cast<int>(i),
            .displayIndex = mainDisplayIndex + 1 + static_cast<int>(i)
        });
    }

    return placements;
}

std::optional<TrackPlacement> GroupMoveSnapEngine::getTrackPlacementById(
    const Timeline& timeline,
    const core::TrackId& trackId
) {
    if (timeline.mainTrack().id() == trackId) {
        return TrackPlacement{
            .trackId = trackId,
            .trackType = timeline.mainTrack().type(),
            .section = GroupTrackSection::Main,
            .sectionIndex = -1,
            .displayIndex = static_cast<int>(timeline.overlayTracks().size())
        };
    }

    const auto& overlays = timeline.overlayTracks();
    for (size_t i = 0; i < overlays.size(); ++i) {
        if (overlays[i].id() == trackId) {
            return TrackPlacement{
                .trackId = trackId,
                .trackType = overlays[i].type(),
                .section = GroupTrackSection::Overlay,
                .sectionIndex = static_cast<int>(i),
                .displayIndex = static_cast<int>(i)
            };
        }
    }

    const auto& audios = timeline.audioTracks();
    for (size_t i = 0; i < audios.size(); ++i) {
        if (audios[i].id() == trackId) {
            return TrackPlacement{
                .trackId = trackId,
                .trackType = audios[i].type(),
                .section = GroupTrackSection::Audio,
                .sectionIndex = static_cast<int>(i),
                .displayIndex = static_cast<int>(overlays.size() + 1 + i)
            };
        }
    }

    return std::nullopt;
}

std::optional<TrackPlacement> GroupMoveSnapEngine::getTrackPlacementByDisplayIndex(
    const Timeline& timeline,
    int displayIndex
) {
    if (displayIndex < 0) {
        return std::nullopt;
    }

    const auto& overlays = timeline.overlayTracks();
    if (displayIndex < static_cast<int>(overlays.size())) {
        const auto& track = overlays[static_cast<size_t>(displayIndex)];
        return TrackPlacement{
            .trackId = track.id(),
            .trackType = track.type(),
            .section = GroupTrackSection::Overlay,
            .sectionIndex = displayIndex,
            .displayIndex = displayIndex
        };
    }

    const int mainDisplayIndex = static_cast<int>(overlays.size());
    if (displayIndex == mainDisplayIndex) {
        return TrackPlacement{
            .trackId = timeline.mainTrack().id(),
            .trackType = timeline.mainTrack().type(),
            .section = GroupTrackSection::Main,
            .sectionIndex = -1,
            .displayIndex = mainDisplayIndex
        };
    }

    const int audioTrackIndex = displayIndex - mainDisplayIndex - 1;
    const auto& audios = timeline.audioTracks();
    if (audioTrackIndex >= 0 && audioTrackIndex < static_cast<int>(audios.size())) {
        const auto& track = audios[static_cast<size_t>(audioTrackIndex)];
        return TrackPlacement{
            .trackId = track.id(),
            .trackType = track.type(),
            .section = GroupTrackSection::Audio,
            .sectionIndex = audioTrackIndex,
            .displayIndex = displayIndex
        };
    }

    return std::nullopt;
}

std::optional<MoveGroupSnapshot> GroupMoveSnapEngine::buildMoveGroup(
    const Timeline& timeline,
    const core::TrackId& anchorTrackId,
    const core::ClipId& anchorClipId,
    const std::vector<std::pair<core::TrackId, core::ClipId>>& selectedElements
) {
    const Track* anchorTrack = timeline.findTrack(anchorTrackId);
    if (!anchorTrack) return std::nullopt;
    const Clip* anchorClip = anchorTrack->findClip(anchorClipId);
    if (!anchorClip) return std::nullopt;
    auto anchorPlacement = getTrackPlacementById(timeline, anchorTrackId);
    if (!anchorPlacement) return std::nullopt;

    std::unordered_set<std::string> seen;
    std::vector<std::pair<core::TrackId, core::ClipId>> orderedRefs;
    orderedRefs.push_back({anchorTrackId, anchorClipId});
    seen.insert(anchorClipId.str());

    for (const auto& ref : selectedElements) {
        if (seen.find(ref.second.str()) == seen.end()) {
            seen.insert(ref.second.str());
            orderedRefs.push_back(ref);
        }
    }

    std::vector<GroupMoveMember> members;
    members.reserve(orderedRefs.size());

    for (const auto& ref : orderedRefs) {
        const Track* trk = timeline.findTrack(ref.first);
        if (!trk) continue;
        const Clip* clp = trk->findClip(ref.second);
        if (!clp) continue;
        auto placement = getTrackPlacementById(timeline, ref.first);
        if (!placement) continue;

        members.push_back(GroupMoveMember{
            .trackId = ref.first,
            .elementId = ref.second,
            .elementType = clp->type(),
            .duration = clp->duration(),
            .timeOffset = clp->startTime() - anchorClip->startTime(),
            .trackSection = placement->section,
            .sectionIndex = placement->sectionIndex,
            .displayIndex = placement->displayIndex
        });
    }

    if (members.empty()) return std::nullopt;

    const auto anchorIt = std::find_if(members.begin(), members.end(), [&](const GroupMoveMember& m) {
        return m.trackId == anchorTrackId && m.elementId == anchorClipId;
    });
    if (anchorIt == members.end()) return std::nullopt;

    return MoveGroupSnapshot{
        .anchor = *anchorIt,
        .members = std::move(members)
    };
}

std::vector<SnapPoint> GroupMoveSnapEngine::buildMoveGroupSnapPoints(
    const MoveGroupSnapshot& group,
    const Timeline& timeline,
    core::TimelineTime playheadTime
) {
    std::unordered_set<std::string> excludeElementIds;
    for (const auto& m : group.members) {
        excludeElementIds.insert(m.elementId.str());
    }

    std::vector<snapping::TimelineSnapPointSource> sources;
    sources.push_back([&timeline, &excludeElementIds]() {
        return snapping::TimelineSnapPointSources::getElementEdgeSnapPoints(timeline, excludeElementIds);
    });
    sources.push_back([playheadTime]() {
        return snapping::TimelineSnapPointSources::getPlayheadSnapPoints(playheadTime);
    });
    sources.push_back([&timeline, &excludeElementIds]() {
        return snapping::TimelineSnapPointSources::getAnimationKeyframeSnapPoints(timeline, excludeElementIds);
    });

    return TimelineSnappingEngine::buildSortedTimelineSnapPoints(sources);
}

SnapGroupResult GroupMoveSnapEngine::snapGroupEdges(
    const MoveGroupSnapshot& group,
    core::TimelineTime anchorStartTime,
    const Timeline& timeline,
    core::TimelineTime playheadTime,
    double zoomLevel,
    const std::optional<std::vector<SnapPoint>>& cachedSnapPoints
) {
    const std::vector<SnapPoint> snapPoints = cachedSnapPoints.has_value()
        ? cachedSnapPoints.value()
        : buildMoveGroupSnapPoints(group, timeline, playheadTime);

    const auto maxSnapDistance = TimelineSnappingEngine::getTimelineSnapThresholdInTicks(zoomLevel);

    int64_t closestSnapDistanceTicks = INT64_MAX;
    core::TimelineTime snappedAnchorStartTime = anchorStartTime;
    std::optional<SnapPoint> bestSnapPoint = std::nullopt;

    for (const auto& member : group.members) {
        const auto memberStartTime = anchorStartTime + member.timeOffset;
        const auto memberStartSnap = TimelineSnappingEngine::resolveSortedTimelineSnap(
            memberStartTime, snapPoints, maxSnapDistance
        );

        if (memberStartSnap.snapPoint.has_value() && memberStartSnap.snapDistanceTicks < closestSnapDistanceTicks) {
            closestSnapDistanceTicks = memberStartSnap.snapDistanceTicks;
            snappedAnchorStartTime = memberStartSnap.snappedTime - member.timeOffset;
            bestSnapPoint = memberStartSnap.snapPoint;
        }

        const auto memberEndTime = memberStartTime + member.duration;
        const auto memberEndSnap = TimelineSnappingEngine::resolveSortedTimelineSnap(
            memberEndTime, snapPoints, maxSnapDistance
        );

        if (memberEndSnap.snapPoint.has_value() && memberEndSnap.snapDistanceTicks < closestSnapDistanceTicks) {
            closestSnapDistanceTicks = memberEndSnap.snapDistanceTicks;
            snappedAnchorStartTime = (memberEndSnap.snappedTime - member.duration) - member.timeOffset;
            bestSnapPoint = memberEndSnap.snapPoint;
        }
    }

    return SnapGroupResult{
        .snappedAnchorStartTime = snappedAnchorStartTime,
        .snapPoint = bestSnapPoint
    };
}

std::optional<GroupMoveResult> GroupMoveSnapEngine::resolveGroupMove(
    const MoveGroupSnapshot& group,
    const Timeline& timeline,
    core::TimelineTime anchorStartTime,
    const GroupMoveTarget& target
) {
    if (std::holds_alternative<NewTracksTarget>(target)) {
        const auto& newTarget = std::get<NewTracksTarget>(target);
        return resolveNewTrackMove(group, timeline, anchorStartTime, newTarget.anchorInsertIndex, newTarget.newTrackIds);
    }

    const auto& existingTarget = std::get<ExistingTrackTarget>(target);
    return resolveExistingTrackMove(group, timeline, anchorStartTime, existingTarget.anchorTargetTrackId);
}

std::optional<GroupMoveResult> GroupMoveSnapEngine::resolveExistingTrackMove(
    const MoveGroupSnapshot& group,
    const Timeline& timeline,
    core::TimelineTime anchorStartTime,
    const core::TrackId& anchorTargetTrackId
) {
    auto anchorTargetPlacement = getTrackPlacementById(timeline, anchorTargetTrackId);
    if (!anchorTargetPlacement) return std::nullopt;

    auto targetTrackIdsByElementId = resolveExistingTrackIdsByElementId(
        group, timeline, anchorTargetPlacement->displayIndex
    );
    if (!targetTrackIdsByElementId) return std::nullopt;

    const auto clampedAnchorStartTime = clampAnchorStartTime(
        group, timeline, anchorStartTime, *targetTrackIdsByElementId
    );

    std::vector<PlannedElementMove> moves;
    moves.reserve(group.members.size());
    std::vector<std::pair<core::TrackId, core::ClipId>> targetSelection;
    targetSelection.reserve(group.members.size());

    for (const auto& member : group.members) {
        const auto it = targetTrackIdsByElementId->find(member.elementId);
        const auto targetTrackId = (it != targetTrackIdsByElementId->end()) ? it->second : member.trackId;
        const auto newStartTime = clampedAnchorStartTime + member.timeOffset;

        moves.push_back(PlannedElementMove{
            .sourceTrackId = member.trackId,
            .targetTrackId = targetTrackId,
            .elementId = member.elementId,
            .newStartTime = newStartTime
        });
        targetSelection.push_back({targetTrackId, member.elementId});
    }

    if (!canApplyMovesToExistingTracks(timeline, moves)) {
        return std::nullopt;
    }

    return GroupMoveResult{
        .moves = std::move(moves),
        .createTracks = {},
        .targetSelection = std::move(targetSelection)
    };
}

std::optional<GroupMoveResult> GroupMoveSnapEngine::resolveNewTrackMove(
    const MoveGroupSnapshot& group,
    const Timeline& timeline,
    core::TimelineTime anchorStartTime,
    int anchorInsertIndex,
    const std::vector<core::TrackId>& newTrackIds
) {
    auto sortedMembers = group.members;
    std::sort(sortedMembers.begin(), sortedMembers.end(), [](const GroupMoveMember& a, const GroupMoveMember& b) {
        return a.displayIndex < b.displayIndex;
    });

    int anchorMemberIndex = -1;
    for (size_t i = 0; i < sortedMembers.size(); ++i) {
        if (sortedMembers[i].elementId == group.anchor.elementId) {
            anchorMemberIndex = static_cast<int>(i);
            break;
        }
    }

    if (anchorMemberIndex < 0 || newTrackIds.size() < sortedMembers.size()) {
        return std::nullopt;
    }

    bool hasAudioMember = false;
    bool hasNonAudioMember = false;
    for (const auto& m : sortedMembers) {
        if (m.trackSection == GroupTrackSection::Audio) {
            hasAudioMember = true;
        } else {
            hasNonAudioMember = true;
        }
    }

    if (hasAudioMember && hasNonAudioMember) {
        return std::nullopt;
    }

    const auto clampedAnchorStartTime = clampAnchorStartTime(
        group, timeline, anchorStartTime, {}
    );

    int blockStartIndex = 0;
    if (hasAudioMember) {
        const int minAudioInsertIndex = static_cast<int>(timeline.overlayTracks().size()) + 1;
        const int maxAudioInsertIndex = minAudioInsertIndex + static_cast<int>(timeline.audioTracks().size());
        const int requestedIndex = anchorInsertIndex - anchorMemberIndex;
        blockStartIndex = std::max(minAudioInsertIndex, std::min(requestedIndex, maxAudioInsertIndex));
    } else {
        const int maxOverlayIndex = static_cast<int>(timeline.overlayTracks().size());
        const int requestedIndex = anchorInsertIndex - anchorMemberIndex;
        blockStartIndex = std::max(0, std::min(requestedIndex, maxOverlayIndex));
    }

    std::vector<PlannedTrackCreation> createTracks;
    createTracks.reserve(sortedMembers.size());
    std::vector<PlannedElementMove> moves;
    moves.reserve(sortedMembers.size());
    std::vector<std::pair<core::TrackId, core::ClipId>> targetSelection;
    targetSelection.reserve(sortedMembers.size());

    for (size_t i = 0; i < sortedMembers.size(); ++i) {
        const auto& member = sortedMembers[i];
        const auto& newId = newTrackIds[i];

        createTracks.push_back(PlannedTrackCreation{
            .id = newId,
            .type = PlacementEngine::getTrackTypeForClipType(member.elementType),
            .index = static_cast<size_t>(blockStartIndex + static_cast<int>(i))
        });

        moves.push_back(PlannedElementMove{
            .sourceTrackId = member.trackId,
            .targetTrackId = newId,
            .elementId = member.elementId,
            .newStartTime = clampedAnchorStartTime + member.timeOffset
        });

        targetSelection.push_back({newId, member.elementId});
    }

    return GroupMoveResult{
        .moves = std::move(moves),
        .createTracks = std::move(createTracks),
        .targetSelection = std::move(targetSelection)
    };
}

core::TimelineTime GroupMoveSnapEngine::clampAnchorStartTime(
    const MoveGroupSnapshot& group,
    const Timeline& timeline,
    core::TimelineTime anchorStartTime,
    const std::map<core::ClipId, core::TrackId>& targetTrackIdsByElementId
) {
    core::TimelineTime minimumAnchorStartTime = core::TimelineTime::zero();
    for (const auto& member : group.members) {
        if (member.timeOffset < core::TimelineTime::zero()) {
            minimumAnchorStartTime = std::max(minimumAnchorStartTime, core::TimelineTime::zero() - member.timeOffset);
        }
    }

    core::TimelineTime clamped = (anchorStartTime < minimumAnchorStartTime)
        ? minimumAnchorStartTime
        : anchorStartTime;

    const auto mainTrackId = timeline.mainTrack().id();
    const GroupMoveMember* memberOnMainTrack = nullptr;
    for (const auto& member : group.members) {
        const auto it = targetTrackIdsByElementId.find(member.elementId);
        if (it != targetTrackIdsByElementId.end() && it->second == mainTrackId) {
            memberOnMainTrack = &member;
            break;
        }
    }

    if (!memberOnMainTrack) {
        return clamped;
    }

    std::unordered_set<std::string> movingElementIds;
    for (const auto& member : group.members) {
        movingElementIds.insert(member.elementId.str());
    }

    const auto requestedMainStartTime = clamped + memberOnMainTrack->timeOffset;

    std::optional<core::TimelineTime> earliestStationaryMainStartTime = std::nullopt;
    for (const auto& clip : timeline.mainTrack().clips()) {
        if (movingElementIds.find(clip.id().str()) == movingElementIds.end()) {
            if (!earliestStationaryMainStartTime.has_value() || clip.startTime() < *earliestStationaryMainStartTime) {
                earliestStationaryMainStartTime = clip.startTime();
            }
        }
    }

    if (!earliestStationaryMainStartTime.has_value() || requestedMainStartTime <= *earliestStationaryMainStartTime) {
        clamped = std::max(minimumAnchorStartTime, core::TimelineTime::zero() - memberOnMainTrack->timeOffset);
    }

    return clamped;
}

std::optional<std::map<core::ClipId, core::TrackId>> GroupMoveSnapEngine::resolveExistingTrackIdsByElementId(
    const MoveGroupSnapshot& group,
    const Timeline& timeline,
    int anchorTargetDisplayIndex
) {
    auto sortedMembers = group.members;
    std::sort(sortedMembers.begin(), sortedMembers.end(), [](const GroupMoveMember& a, const GroupMoveMember& b) {
        return a.displayIndex < b.displayIndex;
    });

    int anchorMemberIndex = -1;
    for (size_t i = 0; i < sortedMembers.size(); ++i) {
        if (sortedMembers[i].elementId == group.anchor.elementId) {
            anchorMemberIndex = static_cast<int>(i);
            break;
        }
    }

    if (anchorMemberIndex < 0) return std::nullopt;

    auto anchorPlacement = getTrackPlacementByDisplayIndex(timeline, anchorTargetDisplayIndex);
    if (!anchorPlacement) return std::nullopt;

    std::map<core::ClipId, core::TrackId> targetTrackIds;
    std::unordered_set<std::string> usedTrackIds;

    targetTrackIds[group.anchor.elementId] = anchorPlacement->trackId;
    usedTrackIds.insert(anchorPlacement->trackId.str());

    int upperBoundaryIndex = anchorTargetDisplayIndex;
    for (int memberIndex = anchorMemberIndex - 1; memberIndex >= 0; --memberIndex) {
        const auto& member = sortedMembers[static_cast<size_t>(memberIndex)];
        auto targetPlacement = findCompatibleTrackPlacement(
            timeline,
            PlacementEngine::getTrackTypeForClipType(member.elementType),
            upperBoundaryIndex - 1,
            -1,
            usedTrackIds
        );
        if (!targetPlacement) return std::nullopt;

        targetTrackIds[member.elementId] = targetPlacement->trackId;
        usedTrackIds.insert(targetPlacement->trackId.str());
        upperBoundaryIndex = targetPlacement->displayIndex;
    }

    int lowerBoundaryIndex = anchorTargetDisplayIndex;
    for (size_t memberIndex = static_cast<size_t>(anchorMemberIndex + 1); memberIndex < sortedMembers.size(); ++memberIndex) {
        const auto& member = sortedMembers[memberIndex];
        auto targetPlacement = findCompatibleTrackPlacement(
            timeline,
            PlacementEngine::getTrackTypeForClipType(member.elementType),
            lowerBoundaryIndex + 1,
            1,
            usedTrackIds
        );
        if (!targetPlacement) return std::nullopt;

        targetTrackIds[member.elementId] = targetPlacement->trackId;
        usedTrackIds.insert(targetPlacement->trackId.str());
        lowerBoundaryIndex = targetPlacement->displayIndex;
    }

    return targetTrackIds;
}

std::optional<TrackPlacement> GroupMoveSnapEngine::findCompatibleTrackPlacement(
    const Timeline& timeline,
    TrackType requiredTrackType,
    int startDisplayIndex,
    int step,
    const std::unordered_set<std::string>& usedTrackIds
) {
    const int totalTracks = static_cast<int>(timeline.overlayTracks().size()) + 1 + static_cast<int>(timeline.audioTracks().size());

    for (int displayIndex = startDisplayIndex; displayIndex >= 0 && displayIndex < totalTracks; displayIndex += step) {
        auto placement = getTrackPlacementByDisplayIndex(timeline, displayIndex);
        if (!placement) continue;

        if (placement->trackType == requiredTrackType && usedTrackIds.find(placement->trackId.str()) == usedTrackIds.end()) {
            return placement;
        }
    }

    return std::nullopt;
}

bool GroupMoveSnapEngine::canApplyMovesToExistingTracks(
    const Timeline& timeline,
    const std::vector<PlannedElementMove>& moves
) {
    std::unordered_set<std::string> movingElementIds;
    for (const auto& m : moves) {
        movingElementIds.insert(m.elementId.str());
    }

    // Group moves by targetTrackId
    std::map<core::TrackId, std::vector<PlannedElementMove>> movesByTargetTrack;
    for (const auto& m : moves) {
        movesByTargetTrack[m.targetTrackId].push_back(m);
    }

    for (const auto& [targetTrackId, targetMoves] : movesByTargetTrack) {
        const Track* targetTrack = timeline.findTrack(targetTrackId);
        if (!targetTrack) return false;

        // Collect timeSpans of targetMoves
        std::vector<PlacementTimeSpan> timeSpans;
        timeSpans.reserve(targetMoves.size());

        for (const auto& m : targetMoves) {
            const Clip* clp = timeline.findClip(m.elementId);
            if (!clp) return false;

            timeSpans.push_back(PlacementTimeSpan{
                .startTime = m.newStartTime,
                .duration = clp->duration(),
                .excludeClipId = m.elementId
            });
        }

        // Check internal overlap among targetMoves
        auto sortedSpans = timeSpans;
        std::sort(sortedSpans.begin(), sortedSpans.end(), [](const PlacementTimeSpan& a, const PlacementTimeSpan& b) {
            return a.startTime < b.startTime;
        });

        for (size_t i = 1; i < sortedSpans.size(); ++i) {
            if (sortedSpans[i - 1].startTime + sortedSpans[i - 1].duration > sortedSpans[i].startTime) {
                return false; // internal collision
            }
        }

        // Check overlap against stationary elements on targetTrack
        Track stationaryTrack(targetTrack->id(), targetTrack->type(), targetTrack->name());
        for (const auto& clip : targetTrack->clips()) {
            if (movingElementIds.find(clip.id().str()) == movingElementIds.end()) {
                stationaryTrack.insertClip(clip);
            }
        }

        if (!PlacementEngine::canPlaceTimeSpansOnTrack(stationaryTrack, timeSpans)) {
            return false;
        }
    }

    return true;
}

} // namespace catchim::editor
