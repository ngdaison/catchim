#include "ClipboardKeyframeEngine.h"
#include <algorithm>

namespace catchim::editor {

PasteKeyframesCommand::PasteKeyframesCommand(
    Timeline& timeline,
    core::TrackId trackId,
    core::ClipId clipId,
    core::TimelineTime targetTime,
    std::vector<KeyframeClipboardItem> items
)
    : timeline_(timeline)
    , trackId_(std::move(trackId))
    , clipId_(std::move(clipId))
    , targetTime_(targetTime)
    , items_(std::move(items))
{
}

bool PasteKeyframesCommand::execute() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip || items_.empty()) {
        return false;
    }

    if (!executed_) {
        savedChannels_ = clip->animationChannels();
    }

    for (const auto& item : items_) {
        core::TimelineTime kfTime = targetTime_ + item.timeOffset;
        if (kfTime < core::TimelineTime(0)) {
            kfTime = core::TimelineTime(0);
        } else if (kfTime > clip->duration()) {
            kfTime = clip->duration();
        }

        AnimationChannel& channel = clip->getOrCreateAnimationChannel(item.propertyPath, item.value);

        Keyframe kf;
        kf.time = kfTime;
        kf.value = item.value;
        kf.interpolation = item.interpolation;
        kf.leftHandle = item.leftHandle;
        kf.rightHandle = item.rightHandle;
        kf.bezierX1 = item.bezierX1;
        kf.bezierY1 = item.bezierY1;
        kf.bezierX2 = item.bezierX2;
        kf.bezierY2 = item.bezierY2;

        channel.addOrUpdateKeyframe(kf);
    }

    executed_ = true;
    return true;
}

bool PasteKeyframesCommand::undo() {
    Clip* clip = timeline_.findClip(clipId_);
    if (!clip) {
        return false;
    }

    clip->animationChannels() = savedChannels_;
    return true;
}

ClipboardKeyframeEngine& ClipboardKeyframeEngine::instance() {
    static ClipboardKeyframeEngine s_instance;
    return s_instance;
}

bool ClipboardKeyframeEngine::canCopy(const std::vector<SelectedKeyframeRef>& selectedKeyframes) const {
    if (selectedKeyframes.empty()) {
        return false;
    }

    const auto& first = selectedKeyframes.front();
    for (const auto& ref : selectedKeyframes) {
        if (ref.trackId != first.trackId || ref.clipId != first.clipId) {
            return false; // Only single source element allowed, matching web spec
        }
    }
    return true;
}

std::optional<KeyframesClipboardEntry> ClipboardKeyframeEngine::copy(
    const Timeline& timeline,
    const std::vector<SelectedKeyframeRef>& selectedKeyframes
) {
    if (!canCopy(selectedKeyframes)) {
        return std::nullopt;
    }

    const auto& first = selectedKeyframes.front();
    const Clip* clip = timeline.findClip(first.clipId);
    if (!clip) {
        return std::nullopt;
    }

    struct RawKeyframeItem {
        std::string propertyPath;
        core::TimelineTime time{0};
        double value{0.0};
        KeyframeInterpolation interpolation{KeyframeInterpolation::Linear};
        KeyframeHandle leftHandle;
        KeyframeHandle rightHandle;
        double bezierX1{0.25};
        double bezierY1{0.1};
        double bezierX2{0.25};
        double bezierY2{1.0};
    };

    std::vector<RawKeyframeItem> rawItems;
    for (const auto& ref : selectedKeyframes) {
        const AnimationChannel* chan = clip->findAnimationChannel(ref.propertyPath);
        if (!chan) continue;

        auto kfOpt = chan->findKeyframeAt(ref.keyframeTime, core::TimelineTime::fromTicks(100));
        if (!kfOpt) continue;

        const auto& kf = *kfOpt;
        RawKeyframeItem raw;
        raw.propertyPath = ref.propertyPath;
        raw.time = kf.time;
        raw.value = kf.value;
        raw.interpolation = kf.interpolation;
        raw.leftHandle = kf.leftHandle;
        raw.rightHandle = kf.rightHandle;
        raw.bezierX1 = kf.bezierX1;
        raw.bezierY1 = kf.bezierY1;
        raw.bezierX2 = kf.bezierX2;
        raw.bezierY2 = kf.bezierY2;
        rawItems.push_back(raw);
    }

    if (rawItems.empty()) {
        return std::nullopt;
    }

    // Determine min time
    core::TimelineTime minTime = rawItems.front().time;
    for (const auto& item : rawItems) {
        if (item.time < minTime) {
            minTime = item.time;
        }
    }

    KeyframesClipboardEntry entry;
    entry.sourceTrackId = first.trackId;
    entry.sourceClipId = first.clipId;

    for (const auto& raw : rawItems) {
        KeyframeClipboardItem item;
        item.propertyPath = raw.propertyPath;
        item.timeOffset = raw.time - minTime;
        item.value = raw.value;
        item.interpolation = raw.interpolation;
        item.leftHandle = raw.leftHandle;
        item.rightHandle = raw.rightHandle;
        item.bezierX1 = raw.bezierX1;
        item.bezierY1 = raw.bezierY1;
        item.bezierX2 = raw.bezierX2;
        item.bezierY2 = raw.bezierY2;
        entry.items.push_back(item);
    }

    // Sort items: timeOffset ascending, then propertyPath
    std::sort(entry.items.begin(), entry.items.end(), [](const KeyframeClipboardItem& a, const KeyframeClipboardItem& b) {
        if (a.timeOffset != b.timeOffset) {
            return a.timeOffset < b.timeOffset;
        }
        return a.propertyPath < b.propertyPath;
    });

    entry_ = entry;
    return entry;
}

std::unique_ptr<PasteKeyframesCommand> ClipboardKeyframeEngine::createPasteCommand(
    Timeline& timeline,
    const core::TrackId& targetTrackId,
    const core::ClipId& targetClipId,
    core::TimelineTime targetTime
) const {
    if (!hasData()) {
        return nullptr;
    }

    return std::make_unique<PasteKeyframesCommand>(
        timeline,
        targetTrackId,
        targetClipId,
        targetTime,
        entry_->items
    );
}

bool ClipboardKeyframeEngine::paste(
    Timeline& timeline,
    CommandHistory& history,
    const core::TrackId& targetTrackId,
    const core::ClipId& targetClipId,
    core::TimelineTime targetTime
) {
    auto cmd = createPasteCommand(timeline, targetTrackId, targetClipId, targetTime);
    if (!cmd) {
        return false;
    }

    return history.execute(std::move(cmd));
}

} // namespace catchim::editor
