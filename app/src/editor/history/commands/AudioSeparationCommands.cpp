#include "editor/history/commands/AudioSeparationCommands.h"
#include "core/logging/Logger.h"

namespace catchim::editor {

// ==================== ExtractSourceAudioCommand ====================

ExtractSourceAudioCommand::ExtractSourceAudioCommand(
    Timeline& timeline,
    core::ClipId videoClipId
) : m_timeline(timeline),
    m_videoClipId(std::move(videoClipId)),
    m_audioClipId(core::ClipId::generate()) {}

bool ExtractSourceAudioCommand::execute() {
    Clip* videoClip = m_timeline.findClip(m_videoClipId);
    if (!videoClip || videoClip->type() != ClipType::Video) {
        LOG_ERROR("ExtractSourceAudioCommand failed: clip is not a valid video clip");
        return false;
    }

    if (videoClip->getParam<bool>("sourceAudioSeparated", false)) {
        LOG_WARN("Source audio is already separated for clip {}", m_videoClipId.str());
        return false;
    }

    m_wasMutedOriginal = videoClip->isMuted();

    // Find or create a suitable audio track
    Track* targetTrack = nullptr;
    for (auto& at : m_timeline.audioTracks()) {
        if (at.canPlace(videoClip->startTime(), videoClip->duration())) {
            targetTrack = &at;
            break;
        }
    }

    if (!targetTrack) {
        targetTrack = &m_timeline.addTrack(
            TrackType::Audio,
            "Audio " + std::to_string(m_timeline.audioTracks().size() + 1)
        );
    }
    m_audioTrackId = targetTrack->id();

    // Create separated audio clip
    Clip audioClip(
        m_audioClipId,
        ClipType::Audio,
        videoClip->name() + " (Audio)",
        videoClip->startTime(),
        videoClip->duration(),
        videoClip->trimStart(),
        videoClip->trimEnd()
    );
    audioClip.setParam<std::string>("linkedVideoClipId", m_videoClipId.str());
    m_extractedAudioClip = audioClip;

    targetTrack->insertClip(std::move(audioClip));

    videoClip->setMuted(true);
    videoClip->setParam<bool>("sourceAudioSeparated", true);
    videoClip->setParam<std::string>("separatedAudioClipId", m_audioClipId.str());
    return true;
}

bool ExtractSourceAudioCommand::undo() {
    m_timeline.removeClip(m_audioClipId);

    Clip* videoClip = m_timeline.findClip(m_videoClipId);
    if (videoClip) {
        videoClip->setMuted(m_wasMutedOriginal);
        videoClip->setParam<bool>("sourceAudioSeparated", false);
        videoClip->setParam<std::string>("separatedAudioClipId", "");
    }
    return true;
}

// ==================== RecoverSourceAudioCommand ====================

RecoverSourceAudioCommand::RecoverSourceAudioCommand(
    Timeline& timeline,
    core::ClipId videoClipId
) : m_timeline(timeline),
    m_videoClipId(std::move(videoClipId)) {}

bool RecoverSourceAudioCommand::execute() {
    Clip* videoClip = m_timeline.findClip(m_videoClipId);
    if (!videoClip || videoClip->type() != ClipType::Video) {
        return false;
    }

    std::string sepIdStr = videoClip->getParam<std::string>("separatedAudioClipId", "");
    if (sepIdStr.empty()) {
        return false;
    }

    m_separatedAudioClipId = core::ClipId(sepIdStr);
    Track* track = m_timeline.findTrackContainingClip(m_separatedAudioClipId);
    if (track) {
        m_audioTrackId = track->id();
    }

    m_removedAudioClip = m_timeline.removeClip(m_separatedAudioClipId);

    videoClip->setMuted(false);
    videoClip->setParam<bool>("sourceAudioSeparated", false);
    videoClip->setParam<std::string>("separatedAudioClipId", "");
    return true;
}

bool RecoverSourceAudioCommand::undo() {
    if (!m_removedAudioClip.has_value()) return false;

    Track* track = m_timeline.findTrack(m_audioTrackId);
    if (!track) return false;

    Clip restored = *m_removedAudioClip;
    track->insertClip(std::move(restored));

    Clip* videoClip = m_timeline.findClip(m_videoClipId);
    if (videoClip) {
        videoClip->setMuted(true);
        videoClip->setParam<bool>("sourceAudioSeparated", true);
        videoClip->setParam<std::string>("separatedAudioClipId", m_separatedAudioClipId.str());
    }
    return true;
}

} // namespace catchim::editor
