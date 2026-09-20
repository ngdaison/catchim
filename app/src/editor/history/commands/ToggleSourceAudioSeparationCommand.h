#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/AudioSeparationEngine.h"
#include "media/MediaAsset.h"
#include <optional>

namespace catchim::editor {

/**
 * @brief Command to toggle source audio separation for a video element.
 * If separated, recovers audio into the source clip.
 * If integrated, extracts source audio onto an audio track with PlacementEngine resolution.
 * Fully reversible via atomic snapshot undo.
 * Corresponds to web/src/commands/timeline/element/toggle-source-audio-separation.ts.
 */
class ToggleSourceAudioSeparationCommand : public EditorCommand {
public:
    ToggleSourceAudioSeparationCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        const media::MediaAsset* mediaAsset = nullptr
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Toggle Source Audio Separation"; }

    [[nodiscard]] bool wasSeparation() const noexcept { return wasSeparation_; }
    [[nodiscard]] const core::ClipId& separatedAudioClipId() const noexcept { return separatedAudioClipId_; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    const media::MediaAsset* mediaAsset_{nullptr};
    std::optional<TimelineTracksSnapshot> savedSnapshot_{std::nullopt};
    bool wasSeparation_{false};
    core::ClipId separatedAudioClipId_{core::ClipId::empty()};
    bool executed_{false};
};

} // namespace catchim::editor
