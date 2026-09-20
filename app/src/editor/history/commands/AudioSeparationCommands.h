#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include "core/ids/Ids.h"
#include <memory>
#include <optional>

namespace catchim::editor {

class ExtractSourceAudioCommand : public EditorCommand {
public:
    ExtractSourceAudioCommand(Timeline& timeline, core::ClipId videoClipId);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Extract Source Audio"; }

    core::ClipId extractedAudioClipId() const noexcept { return m_audioClipId; }

private:
    Timeline& m_timeline;
    core::ClipId m_videoClipId;
    core::ClipId m_audioClipId;
    core::TrackId m_audioTrackId;
    std::optional<Clip> m_extractedAudioClip;
    bool m_wasMutedOriginal{false};
};

class RecoverSourceAudioCommand : public EditorCommand {
public:
    RecoverSourceAudioCommand(Timeline& timeline, core::ClipId videoClipId);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Recover Source Audio"; }

private:
    Timeline& m_timeline;
    core::ClipId m_videoClipId;
    core::ClipId m_separatedAudioClipId;
    core::TrackId m_audioTrackId;
    std::optional<Clip> m_removedAudioClip;
};

} // namespace catchim::editor
