#pragma once

#include "Clip.h"
#include "Timeline.h"
#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <string>

namespace catchim::editor {

struct ElementLocation {
    core::TrackId trackId;
    core::ClipId clipId;

    ElementLocation() = default;
    ElementLocation(core::TrackId tid, core::ClipId cid)
        : trackId(std::move(tid)), clipId(std::move(cid)) {}

    bool operator==(const ElementLocation& other) const noexcept {
        return trackId == other.trackId && clipId == other.clipId;
    }
};

class ElementUtils {
public:
    static bool canElementHaveAudio(ClipType type) noexcept;
    static bool canElementHaveAudio(const Clip& clip) noexcept;

    static bool isVisualElement(ClipType type) noexcept;
    static bool isVisualElement(const Clip& clip) noexcept;

    static bool isMaskableElement(ClipType type) noexcept;
    static bool isMaskableElement(const Clip& clip) noexcept;

    static bool isRetimableElement(ClipType type) noexcept;
    static bool isRetimableElement(const Clip& clip) noexcept;

    static bool canElementBeHidden(ClipType type) noexcept;
    static bool canElementBeHidden(const Clip& clip) noexcept;

    static bool requiresMediaId(ClipType type) noexcept;
    static bool requiresMediaId(const Clip& clip) noexcept;

    /**
     * @brief Finds elements across all tracks present at the specified timeline time.
     * @param strictInterior If true, matches time > start && time < end (web default).
     *                       If false, matches time >= start && time < end (inclusive start).
     */
    static std::vector<ElementLocation> getElementsAtTime(
        const Timeline& timeline,
        core::TimelineTime time,
        bool strictInterior = true
    );

    /**
     * @brief Collects all unique font families defined in Text elements across the timeline.
     */
    static std::vector<std::string> getElementFontFamilies(const Timeline& timeline);
};

} // namespace catchim::editor
