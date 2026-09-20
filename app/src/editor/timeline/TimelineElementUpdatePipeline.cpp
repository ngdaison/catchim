#include "editor/timeline/TimelineElementUpdatePipeline.h"
#include "editor/retime/RetimeEngine.h"
#include <algorithm>

namespace catchim::editor {

bool TimelineElementUpdatePipeline::isRetimableType(ClipType type) noexcept {
    return type == ClipType::Video || type == ClipType::Audio;
}

void TimelineElementUpdatePipeline::clampAnimationsToDuration(Clip& clip, core::TimelineTime duration) {
    if (duration <= core::TimelineTime(0)) {
        for (auto& [prop, channel] : clip.animationChannels()) {
            channel.clear();
        }
        return;
    }

    for (auto& [prop, channel] : clip.animationChannels()) {
        if (channel.keyframes().empty()) {
            continue;
        }

        // Check if any keyframe exceeds the duration
        bool hasExceedingKeyframes = false;
        for (const auto& kf : channel.keyframes()) {
            if (kf.time > duration) {
                hasExceedingKeyframes = true;
                break;
            }
        }

        if (hasExceedingKeyframes) {
            auto [left, right] = channel.splitAt(duration);
            channel = std::move(left);
        }
    }
}

Clip TimelineElementUpdatePipeline::applyElementUpdate(
    const Clip& originalClip,
    const ElementUpdatePatch& patch,
    const ElementUpdateContext& context
) {
    Clip nextElement = originalClip;

    bool hasRetime = patch.retimeRate.has_value();
    bool hasDuration = patch.duration.has_value();
    bool hasStartTime = patch.startTime.has_value();

    // 1. Apply basic metadata and toggle patches
    if (patch.name.has_value()) {
        nextElement.setName(*patch.name);
    }
    if (patch.trimStart.has_value()) {
        nextElement.setTrimStart(*patch.trimStart);
    }
    if (patch.trimEnd.has_value()) {
        nextElement.setTrimEnd(*patch.trimEnd);
    }
    if (patch.sourceDuration.has_value()) {
        nextElement.setSourceDuration(patch.sourceDuration);
    }
    if (patch.hidden.has_value()) {
        nextElement.setHidden(*patch.hidden);
    }
    if (patch.muted.has_value()) {
        nextElement.setMuted(*patch.muted);
    }
    if (patch.params.has_value()) {
        for (auto it = patch.params->begin(); it != patch.params->end(); ++it) {
            nextElement.params()[it.key()] = it.value();
        }
    }

    // 2. Derive Rules: Retime -> Duration derivation
    if (hasRetime && isRetimableType(originalClip.type())) {
        double nextRate = RetimeEngine::clampRate(*patch.retimeRate);
        nextElement.setParam("speed", nextRate);

        core::TimelineTime totalSourceDuration{0};
        if (nextElement.sourceDuration().has_value()) {
            totalSourceDuration = *nextElement.sourceDuration();
        } else {
            double originalRate = originalClip.getParam<double>("speed", 1.0);
            core::TimelineTime originalSourceSpan = RetimeEngine::getSourceTimeAtClipTime(
                originalClip.duration(),
                originalRate
            );
            totalSourceDuration = originalClip.trimStart() + originalSourceSpan + originalClip.trimEnd();
        }

        core::TimelineTime visibleSourceSpan = core::TimelineTime(0);
        if (totalSourceDuration > (nextElement.trimStart() + nextElement.trimEnd())) {
            visibleSourceSpan = totalSourceDuration - nextElement.trimStart() - nextElement.trimEnd();
        }

        core::TimelineTime nextDuration = RetimeEngine::getTimelineDurationForSourceSpan(
            visibleSourceSpan,
            nextRate
        );
        nextElement.setDuration(nextDuration);
        hasDuration = true;
    } else if (hasDuration) {
        nextElement.setDuration(*patch.duration);
    }

    // 3. Enforce Rules:
    // A. Clamping keyframe animations to duration
    if (hasDuration) {
        clampAnimationsToDuration(nextElement, nextElement.duration());
    }

    // B. Clamping startTime & Enforcing Main Track Invariant
    if (hasStartTime) {
        core::TimelineTime requestedStartTime = *patch.startTime;
        if (requestedStartTime < core::TimelineTime(0)) {
            requestedStartTime = core::TimelineTime(0);
        }

        if (context.isMainTrack) {
            core::TimelineTime earliestTime = core::TimelineTime::fromTicks(INT64_MAX);
            bool hasOther = false;

            for (const auto& candidate : context.trackElements) {
                if (candidate && candidate->id() != originalClip.id()) {
                    if (candidate->startTime() < earliestTime) {
                        earliestTime = candidate->startTime();
                    }
                    hasOther = true;
                }
            }

            if (!hasOther || requestedStartTime <= earliestTime) {
                requestedStartTime = core::TimelineTime(0);
            }
        }

        nextElement.setStartTime(requestedStartTime);
    }

    return nextElement;
}

} // namespace catchim::editor
