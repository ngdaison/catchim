#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/ElementUtils.h"
#include "render/masks/FreeformMaskGeometry.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::editor {

/**
 * @brief Command to insert a point on a segment of a clip's freeform mask.
 * Corresponds to web/src/commands/timeline/element/masks/insert-custom-mask-point.ts.
 */
class InsertCustomMaskPointCommand : public EditorCommand {
public:
    InsertCustomMaskPointCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string maskId,
        size_t segmentIndex,
        double t = 0.5
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Insert Mask Point"; }

    const std::string& insertedPointId() const noexcept { return insertedPointId_; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string maskId_;
    size_t segmentIndex_;
    double t_;
    std::string insertedPointId_;
    std::optional<std::vector<MaskInstance>> previousMasks_;
};

/**
 * @brief Command to delete selected points from a clip's freeform mask.
 * Corresponds to web/src/commands/timeline/element/masks/delete-custom-mask-points.ts.
 */
class DeleteCustomMaskPointsCommand : public EditorCommand {
public:
    DeleteCustomMaskPointsCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string maskId,
        std::vector<std::string> pointIds
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Delete Mask Points"; }

    bool didDelete() const noexcept { return didDelete_; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string maskId_;
    std::vector<std::string> pointIds_;
    bool didDelete_{false};
    std::optional<std::vector<MaskInstance>> previousMasks_;
};

/**
 * @brief Command to remove a mask completely from a clip.
 * Corresponds to web/src/commands/timeline/element/masks/remove-mask.ts.
 */
class RemoveMaskCommand : public EditorCommand {
public:
    RemoveMaskCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string maskId
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Remove Mask"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string maskId_;
    std::optional<std::vector<MaskInstance>> previousMasks_;
};

/**
 * @brief Command to toggle the inverted parameter on a clip's mask.
 * Corresponds to web/src/commands/timeline/element/masks/toggle-mask-inverted.ts.
 */
class ToggleMaskInvertedCommand : public EditorCommand {
public:
    ToggleMaskInvertedCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string maskId
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Toggle Mask Inverted"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string maskId_;
    std::optional<std::vector<MaskInstance>> previousMasks_;
};

} // namespace catchim::editor
