#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "subtitles/SubtitleCue.h"
#include <vector>
#include <string>

namespace catchim::editor {

class ImportSubtitlesCommand : public EditorCommand {
public:
    ImportSubtitlesCommand(
        Timeline& timeline,
        std::vector<subtitles::SubtitleCue> cues,
        std::string trackName = "Subtitles"
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override { return "Import Subtitles"; }

    [[nodiscard]] const std::optional<core::TrackId>& createdTrackId() const noexcept {
        return createdTrackId_;
    }

private:
    Timeline& timeline_;
    std::vector<subtitles::SubtitleCue> cues_;
    std::string trackName_;
    std::optional<core::TrackId> createdTrackId_;
    std::optional<Track> savedTrack_;
};

} // namespace catchim::editor
