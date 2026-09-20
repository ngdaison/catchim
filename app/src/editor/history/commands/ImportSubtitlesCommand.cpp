#include "editor/history/commands/ImportSubtitlesCommand.h"

namespace catchim::editor {

ImportSubtitlesCommand::ImportSubtitlesCommand(
    Timeline& timeline,
    std::vector<subtitles::SubtitleCue> cues,
    std::string trackName
)
    : timeline_(timeline)
    , cues_(std::move(cues))
    , trackName_(std::move(trackName))
{
}

bool ImportSubtitlesCommand::execute() {
    if (savedTrack_.has_value()) {
        // Redo: restore track and clips
        Track& newTrack = timeline_.addTrack(savedTrack_->type(), savedTrack_->name());
        createdTrackId_ = newTrack.id();
        for (const auto& clip : savedTrack_->clips()) {
            Clip restored = clip.clone(core::ClipId::generate());
            timeline_.addClip(newTrack.id(), std::move(restored));
        }
        return true;
    }

    // First execution
    Track& track = timeline_.addTrack(TrackType::Text, trackName_);
    createdTrackId_ = track.id();

    for (const auto& cue : cues_) {
        std::string clipName = cue.text.length() > 25 ? cue.text.substr(0, 22) + "..." : cue.text;
        Clip clip(
            core::ClipId::generate(),
            ClipType::Text,
            std::move(clipName),
            cue.startTime,
            cue.duration
        );
        clip.setParam("text", cue.text);
        timeline_.addClip(track.id(), std::move(clip));
    }

    return true;
}

bool ImportSubtitlesCommand::undo() {
    if (!createdTrackId_.has_value()) return false;

    Track* track = timeline_.findTrack(*createdTrackId_);
    if (!track) return false;

    // Save state for redo
    savedTrack_ = *track;

    // Remove track from timeline
    bool removed = timeline_.removeTrack(*createdTrackId_);
    return removed;
}

} // namespace catchim::editor
