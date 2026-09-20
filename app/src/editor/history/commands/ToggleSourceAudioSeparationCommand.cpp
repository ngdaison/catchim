#include "ToggleSourceAudioSeparationCommand.h"
#include "editor/timeline/PlacementEngine.h"

namespace catchim::editor {

ToggleSourceAudioSeparationCommand::ToggleSourceAudioSeparationCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    const media::MediaAsset* mediaAsset
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , mediaAsset_(mediaAsset)
{
}

bool ToggleSourceAudioSeparationCommand::execute() {
    auto* clip = timeline_.findClip(clipId_);
    if (!clip || clip->type() != ClipType::Video) {
        return false;
    }

    savedSnapshot_ = timeline_.createSnapshot();

    if (AudioSeparationEngine::isSourceAudioSeparated(*clip)) {
        // Recover audio: enable audio back on video clip
        clip->setParam<bool>("isSourceAudioEnabled", true);
        clip->setMuted(false);
        wasSeparation_ = false;

        // Remove any corresponding separated audio clip if found
        for (auto& aTrack : timeline_.audioTracks()) {
            for (const auto& c : aTrack.clips()) {
                if (c.mediaId() == clip->mediaId() && c.startTime() == clip->startTime()) {
                    separatedAudioClipId_ = c.id();
                    timeline_.removeClip(c.id());
                    break;
                }
            }
            if (!separatedAudioClipId_.isEmpty()) {
                break;
            }
        }
    } else {
        // Extract audio: create independent audio clip
        if (!AudioSeparationEngine::canExtractSourceAudio(*clip, mediaAsset_)) {
            return false;
        }

        Clip separatedClip = AudioSeparationEngine::buildSeparatedAudioClip(*clip);
        separatedAudioClipId_ = separatedClip.id();

        // Resolve track placement using PlacementEngine
        Track* targetTrack = nullptr;
        for (auto& aTrack : timeline_.audioTracks()) {
            if (PlacementEngine::canPlaceClipOnTrack(aTrack, separatedClip.startTime(), separatedClip.duration())) {
                targetTrack = &aTrack;
                break;
            }
        }

        if (!targetTrack) {
            targetTrack = &timeline_.addTrack(TrackType::Audio, "Audio Track");
        }

        targetTrack->insertClip(std::move(separatedClip));

        // Mark source video clip as having audio separated
        clip->setParam<bool>("isSourceAudioEnabled", false);
        clip->setMuted(true);
        wasSeparation_ = true;
    }

    executed_ = true;
    return true;
}

bool ToggleSourceAudioSeparationCommand::undo() {
    if (!executed_ || !savedSnapshot_.has_value()) {
        return false;
    }

    timeline_.restoreSnapshot(*savedSnapshot_);
    return true;
}

} // namespace catchim::editor
