#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/TimelineSnappingEngine.h"
#include "editor/timeline/TrackCompatibilityEngine.h"
#include <vector>
#include <string>
#include <optional>
#include <variant>
#include <unordered_set>
#include <map>

#include "editor/history/commands/MoveElementsCommand.h"

namespace catchim::editor {

using snapping::SnapPoint;
using snapping::TimelineSnappingEngine;

enum class GroupTrackSection {
    Overlay,
    Main,
    Audio
};

struct TrackPlacement {
    core::TrackId trackId{core::TrackId::empty()};
    TrackType trackType{TrackType::Video};
    GroupTrackSection section{GroupTrackSection::Main};
    int sectionIndex{-1};
    int displayIndex{0};
};

struct GroupMoveMember {
    core::TrackId trackId;
    core::ClipId elementId;
    ClipType elementType{ClipType::Video};
    core::TimelineTime duration{0};
    core::TimelineTime timeOffset{0}; // element.startTime - anchor.startTime
    GroupTrackSection trackSection{GroupTrackSection::Main};
    int sectionIndex{-1};
    int displayIndex{0};
};

struct MoveGroupSnapshot {
    GroupMoveMember anchor;
    std::vector<GroupMoveMember> members;
};

struct PlannedElementMove {
    core::TrackId sourceTrackId;
    core::TrackId targetTrackId;
    core::ClipId elementId;
    core::TimelineTime newStartTime{0};
};

struct GroupMoveResult {
    std::vector<PlannedElementMove> moves;
    std::vector<PlannedTrackCreation> createTracks;
    std::vector<std::pair<core::TrackId, core::ClipId>> targetSelection;
};

struct ExistingTrackTarget {
    core::TrackId anchorTargetTrackId;
};

struct NewTracksTarget {
    int anchorInsertIndex{0};
    std::vector<core::TrackId> newTrackIds;
};

using GroupMoveTarget = std::variant<ExistingTrackTarget, NewTracksTarget>;

struct SnapGroupResult {
    core::TimelineTime snappedAnchorStartTime{0};
    std::optional<SnapPoint> snapPoint{std::nullopt};
};

class GroupMoveSnapEngine {
public:
    static std::vector<TrackPlacement> getDisplayTrackPlacements(const Timeline& timeline);

    static std::optional<TrackPlacement> getTrackPlacementById(
        const Timeline& timeline,
        const core::TrackId& trackId
    );

    static std::optional<TrackPlacement> getTrackPlacementByDisplayIndex(
        const Timeline& timeline,
        int displayIndex
    );

    static std::optional<MoveGroupSnapshot> buildMoveGroup(
        const Timeline& timeline,
        const core::TrackId& anchorTrackId,
        const core::ClipId& anchorClipId,
        const std::vector<std::pair<core::TrackId, core::ClipId>>& selectedElements
    );

    static std::vector<SnapPoint> buildMoveGroupSnapPoints(
        const MoveGroupSnapshot& group,
        const Timeline& timeline,
        core::TimelineTime playheadTime
    );

    static SnapGroupResult snapGroupEdges(
        const MoveGroupSnapshot& group,
        core::TimelineTime anchorStartTime,
        const Timeline& timeline,
        core::TimelineTime playheadTime,
        double zoomLevel,
        const std::optional<std::vector<SnapPoint>>& cachedSnapPoints = std::nullopt
    );

    static std::optional<GroupMoveResult> resolveGroupMove(
        const MoveGroupSnapshot& group,
        const Timeline& timeline,
        core::TimelineTime anchorStartTime,
        const GroupMoveTarget& target
    );

private:
    static std::optional<GroupMoveResult> resolveExistingTrackMove(
        const MoveGroupSnapshot& group,
        const Timeline& timeline,
        core::TimelineTime anchorStartTime,
        const core::TrackId& anchorTargetTrackId
    );

    static std::optional<GroupMoveResult> resolveNewTrackMove(
        const MoveGroupSnapshot& group,
        const Timeline& timeline,
        core::TimelineTime anchorStartTime,
        int anchorInsertIndex,
        const std::vector<core::TrackId>& newTrackIds
    );

    static core::TimelineTime clampAnchorStartTime(
        const MoveGroupSnapshot& group,
        const Timeline& timeline,
        core::TimelineTime anchorStartTime,
        const std::map<core::ClipId, core::TrackId>& targetTrackIdsByElementId
    );

    static std::optional<std::map<core::ClipId, core::TrackId>> resolveExistingTrackIdsByElementId(
        const MoveGroupSnapshot& group,
        const Timeline& timeline,
        int anchorTargetDisplayIndex
    );

    static std::optional<TrackPlacement> findCompatibleTrackPlacement(
        const Timeline& timeline,
        TrackType requiredTrackType,
        int startDisplayIndex,
        int step,
        const std::unordered_set<std::string>& usedTrackIds
    );

    static bool canApplyMovesToExistingTracks(
        const Timeline& timeline,
        const std::vector<PlannedElementMove>& moves
    );
};

} // namespace catchim::editor
