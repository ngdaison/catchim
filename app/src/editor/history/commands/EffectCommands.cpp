#include "EffectCommands.h"
#include "core/ids/Ids.h"
#include <algorithm>

namespace catchim::editor {

// ============================================================================
// AddClipEffectCommand
// ============================================================================

AddClipEffectCommand::AddClipEffectCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string effectType,
    nlohmann::json defaultParams
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , effectType_(std::move(effectType))
    , defaultParams_(std::move(defaultParams))
{
}

bool AddClipEffectCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isVisualElement(*clip)) {
        return false;
    }

    previousEffects_ = clip->effects();
    createdEffectId_ = "eff_" + core::EffectId::generate().str();

    EffectInstance instance;
    instance.id = createdEffectId_;
    instance.type = effectType_;
    instance.params = defaultParams_;
    instance.enabled = true;

    clip->effects().push_back(std::move(instance));
    return true;
}

bool AddClipEffectCommand::undo() {
    if (!previousEffects_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setEffects(std::move(*previousEffects_));
    previousEffects_.reset();
    return true;
}

// ============================================================================
// RemoveClipEffectCommand
// ============================================================================

RemoveClipEffectCommand::RemoveClipEffectCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string effectId
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , effectId_(std::move(effectId))
{
}

bool RemoveClipEffectCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isVisualElement(*clip)) {
        return false;
    }

    auto& effs = clip->effects();
    auto it = std::find_if(effs.begin(), effs.end(), [&](const EffectInstance& e) {
        return e.id == effectId_;
    });
    if (it == effs.end()) {
        return false;
    }

    previousEffects_ = effs;
    effs.erase(it);
    return true;
}

bool RemoveClipEffectCommand::undo() {
    if (!previousEffects_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setEffects(std::move(*previousEffects_));
    previousEffects_.reset();
    return true;
}

// ============================================================================
// ToggleClipEffectCommand
// ============================================================================

ToggleClipEffectCommand::ToggleClipEffectCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string effectId
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , effectId_(std::move(effectId))
{
}

bool ToggleClipEffectCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isVisualElement(*clip)) {
        return false;
    }

    auto& effs = clip->effects();
    auto it = std::find_if(effs.begin(), effs.end(), [&](const EffectInstance& e) {
        return e.id == effectId_;
    });
    if (it == effs.end()) {
        return false;
    }

    previousEffects_ = effs;
    it->enabled = !it->enabled;
    return true;
}

bool ToggleClipEffectCommand::undo() {
    if (!previousEffects_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setEffects(std::move(*previousEffects_));
    previousEffects_.reset();
    return true;
}

// ============================================================================
// ReorderClipEffectsCommand
// ============================================================================

ReorderClipEffectsCommand::ReorderClipEffectsCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    size_t fromIndex,
    size_t toIndex
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , fromIndex_(fromIndex)
    , toIndex_(toIndex)
{
}

bool ReorderClipEffectsCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isVisualElement(*clip)) {
        return false;
    }

    auto& effs = clip->effects();
    if (fromIndex_ >= effs.size() || toIndex_ >= effs.size() || fromIndex_ == toIndex_) {
        return false;
    }

    previousEffects_ = effs;
    auto item = std::move(effs[fromIndex_]);
    effs.erase(effs.begin() + fromIndex_);
    effs.insert(effs.begin() + toIndex_, std::move(item));
    return true;
}

bool ReorderClipEffectsCommand::undo() {
    if (!previousEffects_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setEffects(std::move(*previousEffects_));
    previousEffects_.reset();
    return true;
}

// ============================================================================
// UpdateClipEffectParamsCommand
// ============================================================================

UpdateClipEffectParamsCommand::UpdateClipEffectParamsCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    std::string effectId,
    nlohmann::json patchParams
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , effectId_(std::move(effectId))
    , patchParams_(std::move(patchParams))
{
}

bool UpdateClipEffectParamsCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || !ElementUtils::isVisualElement(*clip)) {
        return false;
    }

    auto& effs = clip->effects();
    auto it = std::find_if(effs.begin(), effs.end(), [&](EffectInstance& e) {
        return e.id == effectId_;
    });
    if (it == effs.end()) {
        return false;
    }

    previousEffects_ = effs;
    if (patchParams_.is_object()) {
        for (const auto& [k, v] : patchParams_.items()) {
            it->params[k] = v;
        }
    }
    return true;
}

bool UpdateClipEffectParamsCommand::undo() {
    if (!previousEffects_.has_value()) {
        return false;
    }
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }
    clip->setEffects(std::move(*previousEffects_));
    previousEffects_.reset();
    return true;
}

} // namespace catchim::editor
