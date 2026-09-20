#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/animation/Keyframe.h"
#include "editor/animation/AnimationChannel.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <optional>

namespace catchim::editor {

/**
 * @brief Command to insert or update a keyframe on a clip's animation channel.
 * Corresponds to web/src/commands/timeline/element/keyframes/upsert-keyframe.ts.
 */
class UpsertKeyframeCommand : public EditorCommand {
public:
    UpsertKeyframeCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string propertyPath,
        core::TimelineTime time,
        double value,
        KeyframeInterpolation interpolation = KeyframeInterpolation::Linear,
        KeyframeHandle leftHandle = {-0.2, 0.0},
        KeyframeHandle rightHandle = {0.2, 0.0},
        double bezierX1 = 0.25,
        double bezierY1 = 0.1,
        double bezierX2 = 0.25,
        double bezierY2 = 1.0
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Upsert Keyframe"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string propertyPath_;
    core::TimelineTime time_;
    double value_;
    KeyframeInterpolation interpolation_;
    KeyframeHandle leftHandle_;
    KeyframeHandle rightHandle_;
    double bezierX1_;
    double bezierY1_;
    double bezierX2_;
    double bezierY2_;

    std::optional<AnimationChannel> savedChannel_{std::nullopt};
    bool hadChannel_{false};
    bool executed_{false};
};

/**
 * @brief Command to remove a keyframe at a specified time on a clip's animation channel.
 * Corresponds to web/src/commands/timeline/element/keyframes/remove-keyframe.ts.
 */
class RemoveKeyframeCommand : public EditorCommand {
public:
    RemoveKeyframeCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string propertyPath,
        core::TimelineTime time
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Remove Keyframe"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string propertyPath_;
    core::TimelineTime time_;

    std::optional<AnimationChannel> savedChannel_{std::nullopt};
    bool hadChannel_{false};
    bool executed_{false};
};

/**
 * @brief Command to retime (shift timestamp of) an existing keyframe.
 * Corresponds to web/src/commands/timeline/element/keyframes/retime-keyframe.ts.
 */
class RetimeKeyframeCommand : public EditorCommand {
public:
    RetimeKeyframeCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string propertyPath,
        core::TimelineTime oldTime,
        core::TimelineTime newTime
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Retime Keyframe"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string propertyPath_;
    core::TimelineTime oldTime_;
    core::TimelineTime newTime_;

    std::optional<AnimationChannel> savedChannel_{std::nullopt};
    bool hadChannel_{false};
    bool executed_{false};
};

/**
 * @brief Command to update curve interpolation and bezier handles on an existing keyframe.
 * Corresponds to web/src/commands/timeline/element/keyframes/update-scalar-keyframe-curve.ts.
 */
class UpdateKeyframeCurveCommand : public EditorCommand {
public:
    UpdateKeyframeCurveCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string propertyPath,
        core::TimelineTime time,
        KeyframeInterpolation interpolation,
        KeyframeHandle leftHandle,
        KeyframeHandle rightHandle,
        double bezierX1 = 0.25,
        double bezierY1 = 0.1,
        double bezierX2 = 0.25,
        double bezierY2 = 1.0
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Update Keyframe Curve"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string propertyPath_;
    core::TimelineTime time_;
    KeyframeInterpolation interpolation_;
    KeyframeHandle leftHandle_;
    KeyframeHandle rightHandle_;
    double bezierX1_;
    double bezierY1_;
    double bezierX2_;
    double bezierY2_;

    std::optional<AnimationChannel> savedChannel_{std::nullopt};
    bool hadChannel_{false};
    bool executed_{false};
};

} // namespace catchim::editor
