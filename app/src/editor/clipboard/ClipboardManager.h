#pragma once

#include "editor/timeline/Clip.h"
#include "editor/timeline/Timeline.h"
#include "editor/history/CommandHistory.h"
#include <vector>
#include <memory>

namespace catchim::editor {

struct ClipboardItem {
    Clip clip;
    core::TimelineTime relativeOffset{0};
    TrackType trackType{TrackType::Video};

    ClipboardItem(Clip c, core::TimelineTime offset, TrackType tt)
        : clip(std::move(c)), relativeOffset(offset), trackType(tt) {}
};

class PasteClipsCommand : public EditorCommand {
public:
    PasteClipsCommand(Timeline& timeline, std::vector<std::pair<core::TrackId, Clip>> clipsToInsert);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Paste Clips"; }

    const std::vector<core::ClipId>& pastedClipIds() const noexcept { return m_pastedIds; }

private:
    Timeline& m_timeline;
    std::vector<std::pair<core::TrackId, Clip>> m_clipsToInsert;
    std::vector<core::ClipId> m_pastedIds;
};

class ClipboardManager {
public:
    static ClipboardManager& instance();

    bool copy(const Timeline& timeline, const std::vector<core::ClipId>& clipIds);
    bool paste(Timeline& timeline, CommandHistory& history, core::TimelineTime targetTime);

    bool hasData() const noexcept { return !m_items.empty(); }
    size_t count() const noexcept { return m_items.size(); }
    void clear() noexcept { m_items.clear(); }

    const std::vector<core::ClipId>& lastPastedIds() const noexcept { return m_lastPastedIds; }

private:
    ClipboardManager() = default;

    std::vector<ClipboardItem> m_items;
    std::vector<core::ClipId> m_lastPastedIds;
};

} // namespace catchim::editor
