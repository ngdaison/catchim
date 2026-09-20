#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/ElementUtils.h"
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::editor {

/**
 * @brief Command to attach a new visual effect instance to a visual clip.
 * Corresponds to web/src/commands/timeline/element/effects/add-effect.ts.
 */
class AddClipEffectCommand : public EditorCommand {
public:
    AddClipEffectCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string effectType,
        nlohmann::json defaultParams = nlohmann::json::object()
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Add Effect"; }

    const std::string& createdEffectId() const noexcept { return createdEffectId_; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string effectType_;
    nlohmann::json defaultParams_;
    std::string createdEffectId_;
    std::optional<std::vector<EffectInstance>> previousEffects_;
};

/**
 * @brief Command to remove an effect instance from a clip.
 * Corresponds to web/src/commands/timeline/element/effects/remove-effect.ts.
 */
class RemoveClipEffectCommand : public EditorCommand {
public:
    RemoveClipEffectCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string effectId
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Remove Effect"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string effectId_;
    std::optional<std::vector<EffectInstance>> previousEffects_;
};

/**
 * @brief Command to toggle enabled/disabled state of an effect instance.
 * Corresponds to web/src/commands/timeline/element/effects/toggle-effect.ts.
 */
class ToggleClipEffectCommand : public EditorCommand {
public:
    ToggleClipEffectCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string effectId
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Toggle Effect"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string effectId_;
    std::optional<std::vector<EffectInstance>> previousEffects_;
};

/**
 * @brief Command to reorder effect stack positions on an element.
 * Corresponds to web/src/commands/timeline/element/effects/reorder-effect.ts.
 */
class ReorderClipEffectsCommand : public EditorCommand {
public:
    ReorderClipEffectsCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        size_t fromIndex,
        size_t toIndex
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Reorder Effects"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    size_t fromIndex_;
    size_t toIndex_;
    std::optional<std::vector<EffectInstance>> previousEffects_;
};

/**
 * @brief Command to update / patch parameters of an effect instance.
 * Corresponds to web/src/commands/timeline/element/effects/update-effect-params.ts.
 */
class UpdateClipEffectParamsCommand : public EditorCommand {
public:
    UpdateClipEffectParamsCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        std::string effectId,
        nlohmann::json patchParams
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Update Effect Params"; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    std::string effectId_;
    nlohmann::json patchParams_;
    std::optional<std::vector<EffectInstance>> previousEffects_;
};

} // namespace catchim::editor
