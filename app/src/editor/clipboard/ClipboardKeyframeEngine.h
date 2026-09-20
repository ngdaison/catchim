#pragma once

#include "editor/animation/Keyframe.h"
#include "editor/animation/AnimationChannel.h"
#include "editor/timeline/Timeline.h"
#include "editor/history/Command.h"
#include "editor/history/CommandHistory.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <string>
#include <optional>
#include <memory>
#include <unordered_map>

namespace catchim::editor {

struct SelectedKeyframeRef {
    core::TrackId trackId;
    core::ClipId clipId;
    std::string propertyPath;
    core::TimelineTime keyframeTime{0};

    bool operator==(const SelectedKeyframeRef& other) const noexcept {
        return trackId == other.trackId &&
               clipId == other.clipId &&
               propertyPath == other.propertyPath &&
               keyframeTime == other.keyframeTime;
    }
};

struct KeyframeClipboardItem {
    std::string propertyPath;
    core::TimelineTime timeOffset{0}; // Offset relative to the earliest keyframe copied
    double value{0.0};
    KeyframeInterpolation interpolation{KeyframeInterpolation::Linear};
    KeyframeHandle leftHandle{-0.2, 0.0};
    KeyframeHandle rightHandle{0.2, 0.0};
    double bezierX1{0.25};
    double bezierY1{0.1};
    double bezierX2{0.25};
    double bezierY2{1.0};
};

struct KeyframesClipboardEntry {
    core::TrackId sourceTrackId;
    core::ClipId sourceClipId;
    std::vector<KeyframeClipboardItem> items;
};

class PasteKeyframesCommand : public EditorCommand {
public:
    PasteKeyframesCommand(
        Timeline& timeline,
        core::TrackId trackId,
        core::ClipId clipId,
        core::TimelineTime targetTime,
        std::vector<KeyframeClipboardItem> items
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Paste Keyframes"; }

    const std::vector<KeyframeClipboardItem>& items() const noexcept { return items_; }

private:
    Timeline& timeline_;
    core::TrackId trackId_;
    core::ClipId clipId_;
    core::TimelineTime targetTime_;
    std::vector<KeyframeClipboardItem> items_;
    std::unordered_map<std::string, AnimationChannel> savedChannels_;
    bool executed_{false};
};

class ClipboardKeyframeEngine {
public:
    static ClipboardKeyframeEngine& instance();

    bool canCopy(const std::vector<SelectedKeyframeRef>& selectedKeyframes) const;

    std::optional<KeyframesClipboardEntry> copy(
        const Timeline& timeline,
        const std::vector<SelectedKeyframeRef>& selectedKeyframes
    );

    bool paste(
        Timeline& timeline,
        CommandHistory& history,
        const core::TrackId& targetTrackId,
        const core::ClipId& targetClipId,
        core::TimelineTime targetTime
    );

    std::unique_ptr<PasteKeyframesCommand> createPasteCommand(
        Timeline& timeline,
        const core::TrackId& targetTrackId,
        const core::ClipId& targetClipId,
        core::TimelineTime targetTime
    ) const;

    bool hasData() const noexcept { return entry_.has_value() && !entry_->items.empty(); }
    const std::optional<KeyframesClipboardEntry>& getClipboard() const noexcept { return entry_; }
    void setClipboard(KeyframesClipboardEntry entry) { entry_ = std::move(entry); }
    void clear() noexcept { entry_.reset(); }

private:
    ClipboardKeyframeEngine() = default;
    std::optional<KeyframesClipboardEntry> entry_{std::nullopt};
};

} // namespace catchim::editor
