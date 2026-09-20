#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <unordered_set>
#include <nlohmann/json.hpp>

namespace catchim::editor {

struct ElementUpdatePatch {
    std::optional<std::string> name{std::nullopt};
    std::optional<core::TimelineTime> startTime{std::nullopt};
    std::optional<core::TimelineTime> duration{std::nullopt};
    std::optional<core::TimelineTime> trimStart{std::nullopt};
    std::optional<core::TimelineTime> trimEnd{std::nullopt};
    std::optional<core::TimelineTime> sourceDuration{std::nullopt};
    std::optional<double> retimeRate{std::nullopt};
    std::optional<bool> maintainPitch{std::nullopt};
    std::optional<bool> hidden{std::nullopt};
    std::optional<bool> muted{std::nullopt};
    std::optional<nlohmann::json> params{std::nullopt};
};

struct ElementUpdateContext {
    std::string trackId;
    bool isMainTrack{false};
    std::vector<std::shared_ptr<Clip>> trackElements;
};

class TimelineElementUpdatePipeline {
public:
    // Determines whether a clip type supports retime / speed alterations (Video, Audio)
    static bool isRetimableType(ClipType type) noexcept;

    // Clamps all animation channels within the clip to not exceed the specified duration
    static void clampAnimationsToDuration(Clip& clip, core::TimelineTime duration);

    // Core rule-based pipeline that derives dependent values (retime -> duration)
    // and enforces invariants (non-negative startTime, main track zero-start, animation clamping).
    static Clip applyElementUpdate(
        const Clip& originalClip,
        const ElementUpdatePatch& patch,
        const ElementUpdateContext& context
    );
};

} // namespace catchim::editor
