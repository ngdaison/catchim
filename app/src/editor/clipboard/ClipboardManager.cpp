#include "editor/clipboard/ClipboardManager.h"
#include "core/logging/Logger.h"
#include <limits>

namespace catchim::editor {

// ==================== PasteClipsCommand ====================

PasteClipsCommand::PasteClipsCommand(
    Timeline& timeline,
    std::vector<std::pair<core::TrackId, Clip>> clipsToInsert
) : m_timeline(timeline),
    m_clipsToInsert(std::move(clipsToInsert)) {}

bool PasteClipsCommand::execute() {
    m_pastedIds.clear();
    for (const auto& [trackId, clip] : m_clipsToInsert) {
        Track* track = m_timeline.findTrack(trackId);
        if (track) {
            m_pastedIds.push_back(clip.id());
            track->insertClip(clip);
        }
    }
    return !m_pastedIds.empty();
}

bool PasteClipsCommand::undo() {
    for (const auto& cid : m_pastedIds) {
        m_timeline.removeClip(cid);
    }
    return true;
}

// ==================== ClipboardManager ====================

ClipboardManager& ClipboardManager::instance() {
    static ClipboardManager s_instance;
    return s_instance;
}

bool ClipboardManager::copy(const Timeline& timeline, const std::vector<core::ClipId>& clipIds) {
    if (clipIds.empty()) {
        return false;
    }

    core::TimelineTime minStart = core::TimelineTime::fromTicks(std::numeric_limits<int64_t>::max());
    std::vector<std::pair<const Clip*, TrackType>> validClips;

    for (const auto& cid : clipIds) {
        const Track* track = timeline.findTrackContainingClip(cid);
        const Clip* clip = timeline.findClip(cid);
        if (track && clip) {
            validClips.push_back({clip, track->type()});
            if (clip->startTime() < minStart) {
                minStart = clip->startTime();
            }
        }
    }

    if (validClips.empty()) {
        return false;
    }

    m_items.clear();
    for (const auto& [c, tt] : validClips) {
        m_items.emplace_back(*c, c->startTime() - minStart, tt);
    }

    LOG_INFO("Clipboard: copied {} clips", m_items.size());
    return true;
}

bool ClipboardManager::paste(Timeline& timeline, CommandHistory& history, core::TimelineTime targetTime) {
    if (m_items.empty()) {
        LOG_WARN("Clipboard paste attempted but clipboard is empty");
        return false;
    }

    std::vector<std::pair<core::TrackId, Clip>> clipsToInsert;

    for (const auto& item : m_items) {
        core::TimelineTime pasteStart = targetTime + item.relativeOffset;
        Clip newClip = item.clip.clone(core::ClipId::generate());
        newClip.setStartTime(pasteStart);

        Track* targetTrack = nullptr;
        for (auto* t : timeline.allTracks()) {
            if (t->type() == item.trackType && t->canPlace(pasteStart, newClip.duration())) {
                targetTrack = t;
                break;
            }
        }

        if (!targetTrack) {
            if (item.trackType == TrackType::Audio) {
                targetTrack = &timeline.addTrack(
                    TrackType::Audio,
                    "Audio " + std::to_string(timeline.audioTracks().size() + 1)
                );
            } else {
                targetTrack = &timeline.addTrack(
                    item.trackType,
                    "Overlay " + std::to_string(timeline.overlayTracks().size() + 1)
                );
            }
        }

        clipsToInsert.push_back({targetTrack->id(), std::move(newClip)});
    }

    auto cmd = std::make_unique<PasteClipsCommand>(timeline, std::move(clipsToInsert));
    auto* cmdPtr = cmd.get();

    if (history.execute(std::move(cmd))) {
        m_lastPastedIds = cmdPtr->pastedClipIds();
        LOG_INFO("Clipboard: pasted {} clips successfully", m_lastPastedIds.size());
        return true;
    }

    return false;
}

} // namespace catchim::editor
