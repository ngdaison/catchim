#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/animation/Keyframe.h"
#include "editor/animation/AnimationChannel.h"
#include "editor/animation/EffectParamAnimationEngine.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <string>
#include <optional>

namespace catchim::editor {

/**
 * @brief Command to insert or update a keyframe on an effect parameter animation channel.
 * Corresponds to web/src/commands/timeline/element/keyframes/upsert-effect-param-keyframe.ts.
 */
class UpsertEffectParamKeyframeCommand : public EditorCommand {
public:
    UpsertEffectParamKeyframeCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string effectId,
        std::string paramKey,
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
    std::string name() const override { return "Upsert Effect Param Keyframe"; }

    const std::string& propertyPath() const noexcept { return propertyPath_; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string effectId_;
    std::string paramKey_;
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
 * @brief Command to remove a keyframe from an effect parameter channel.
 * Corresponds to web/src/commands/timeline/element/keyframes/remove-effect-param-keyframe.ts.
 */
class RemoveEffectParamKeyframeCommand : public EditorCommand {
public:
    RemoveEffectParamKeyframeCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string effectId,
        std::string paramKey,
        core::TimelineTime time,
        core::TimelineTime threshold = core::TimelineTime::fromTicks(100)
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Remove Effect Param Keyframe"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string effectId_;
    std::string paramKey_;
    std::string propertyPath_;
    core::TimelineTime time_;
    core::TimelineTime threshold_;

    std::optional<AnimationChannel> savedChannel_{std::nullopt};
    bool executed_{false};
};

} // namespace catchim::editor
