#include "editor/history/commands/SplitElementsCommand.h"
#include "editor/retime/RetimeEngine.h"
#include <algorithm>

namespace catchim::editor {

SplitElementsCommand::SplitElementsCommand(
    Timeline& timeline,
    std::vector<ElementLocation> elements,
    core::TimelineTime splitTime,
    RetainSide retainSide
)
    : timeline_(timeline),
      elements_(std::move(elements)),
      splitTime_(splitTime),
      retainSide_(retainSide) {}

std::string SplitElementsCommand::name() const {
    switch (retainSide_) {
        case RetainSide::Left:
            return "Split Elements (Retain Left)";
        case RetainSide::Right:
            return "Split Elements (Retain Right)";
        case RetainSide::Both:
        default:
            return "Split Elements";
    }
}

static void splitTrackElements(
    Track& track,
    const std::vector<ElementLocation>& targets,
    core::TimelineTime splitTime,
    RetainSide retainSide,
    std::vector<ElementLocation>& outRightElements
) {
    std::vector<core::ClipId> targetClipIds;
    for (const auto& loc : targets) {
        if (loc.trackId == track.id()) {
            targetClipIds.push_back(loc.clipId);
        }
    }

    if (targetClipIds.empty()) {
        return;
    }

    std::vector<Clip> newClips;
    newClips.reserve(track.clips().size() + targetClipIds.size());

    for (auto& clip : track.clips()) {
        bool isTarget = std::find(targetClipIds.begin(), targetClipIds.end(), clip.id()) != targetClipIds.end();
        if (!isTarget || splitTime <= clip.startTime() || splitTime >= clip.endTime()) {
            newClips.push_back(std::move(clip));
            continue;
        }

        core::TimelineTime relativeTime = splitTime - clip.startTime();
        core::TimelineTime leftVisibleDuration = relativeTime;
        core::TimelineTime rightVisibleDuration = clip.duration() - relativeTime;

        double rate = 1.0;
        if (clip.params().contains("retime") && clip.params()["retime"].is_object()) {
            const auto& r = clip.params()["retime"];
            if (r.contains("rate") && r["rate"].is_number()) {
                rate = r["rate"].get<double>();
            }
        }

        core::TimelineTime leftSourceSpan = RetimeEngine::getSourceTimeAtClipTime(leftVisibleDuration, rate);
        core::TimelineTime totalSourceSpan = RetimeEngine::getSourceTimeAtClipTime(clip.duration(), rate);
        core::TimelineTime rightSourceSpan = totalSourceSpan - leftSourceSpan;

        std::unordered_map<std::string, AnimationChannel> leftAnims;
        std::unordered_map<std::string, AnimationChannel> rightAnims;
        for (const auto& [prop, chan] : clip.animationChannels()) {
            auto [lChan, rChan] = chan.splitAt(relativeTime);
            leftAnims[prop] = std::move(lChan);
            rightAnims[prop] = std::move(rChan);
        }

        core::TimelineTime leftTrimEnd = clip.trimEnd() + rightSourceSpan;
        core::TimelineTime rightTrimStart = clip.trimStart() + leftSourceSpan;

        if (retainSide == RetainSide::Left) {
            clip.setDuration(leftVisibleDuration);
            clip.setTrimEnd(leftTrimEnd);
            clip.setName(clip.name() + " (left)");
            clip.animationChannels() = std::move(leftAnims);
            newClips.push_back(std::move(clip));
        } else if (retainSide == RetainSide::Right) {
            core::ClipId newId = core::ClipId::generate();
            Clip rightClip = clip.clone(newId);
            rightClip.setStartTime(splitTime);
            rightClip.setDuration(rightVisibleDuration);
            rightClip.setTrimStart(rightTrimStart);
            rightClip.setName(clip.name() + " (right)");
            rightClip.animationChannels() = std::move(rightAnims);

            outRightElements.push_back(ElementLocation{track.id(), newId});
            newClips.push_back(std::move(rightClip));
        } else {
            // RetainSide::Both
            core::ClipId rightId = core::ClipId::generate();
            Clip rightClip = clip.clone(rightId);
            rightClip.setStartTime(splitTime);
            rightClip.setDuration(rightVisibleDuration);
            rightClip.setTrimStart(rightTrimStart);
            rightClip.setName(clip.name() + " (right)");
            rightClip.animationChannels() = std::move(rightAnims);

            clip.setDuration(leftVisibleDuration);
            clip.setTrimEnd(leftTrimEnd);
            clip.setName(clip.name() + " (left)");
            clip.animationChannels() = std::move(leftAnims);

            outRightElements.push_back(ElementLocation{track.id(), rightId});
            newClips.push_back(std::move(clip));
            newClips.push_back(std::move(rightClip));
        }
    }

    track.clips() = std::move(newClips);
    track.sortClips();
}

bool SplitElementsCommand::execute() {
    savedState_ = timeline_.createSnapshot();
    rightSideElements_.clear();

    for (auto& track : timeline_.overlayTracks()) {
        splitTrackElements(track, elements_, splitTime_, retainSide_, rightSideElements_);
    }

    splitTrackElements(timeline_.mainTrack(), elements_, splitTime_, retainSide_, rightSideElements_);

    for (auto& track : timeline_.audioTracks()) {
        splitTrackElements(track, elements_, splitTime_, retainSide_, rightSideElements_);
    }

    return true;
}

bool SplitElementsCommand::undo() {
    if (!savedState_) {
        return false;
    }
    timeline_.restoreSnapshot(*savedState_);
    return true;
}

} // namespace catchim::editor
