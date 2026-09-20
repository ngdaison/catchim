#include "ElementUtils.h"
#include <set>
#include <algorithm>

namespace catchim::editor {

bool ElementUtils::canElementHaveAudio(ClipType type) noexcept {
    return type == ClipType::Audio || type == ClipType::Video;
}

bool ElementUtils::canElementHaveAudio(const Clip& clip) noexcept {
    return canElementHaveAudio(clip.type());
}

bool ElementUtils::isVisualElement(ClipType type) noexcept {
    return type == ClipType::Video ||
           type == ClipType::Image ||
           type == ClipType::Text ||
           type == ClipType::Sticker ||
           type == ClipType::Graphic ||
           type == ClipType::Effect;
}

bool ElementUtils::isVisualElement(const Clip& clip) noexcept {
    return isVisualElement(clip.type());
}

bool ElementUtils::isMaskableElement(ClipType type) noexcept {
    return type == ClipType::Video ||
           type == ClipType::Image ||
           type == ClipType::Text ||
           type == ClipType::Graphic ||
           type == ClipType::Sticker;
}

bool ElementUtils::isMaskableElement(const Clip& clip) noexcept {
    return isMaskableElement(clip.type());
}

bool ElementUtils::isRetimableElement(ClipType type) noexcept {
    return type == ClipType::Video || type == ClipType::Audio;
}

bool ElementUtils::isRetimableElement(const Clip& clip) noexcept {
    return isRetimableElement(clip.type());
}

bool ElementUtils::canElementBeHidden(ClipType type) noexcept {
    return isVisualElement(type);
}

bool ElementUtils::canElementBeHidden(const Clip& clip) noexcept {
    return isVisualElement(clip.type());
}

bool ElementUtils::requiresMediaId(ClipType type) noexcept {
    return type == ClipType::Video ||
           type == ClipType::Audio ||
           type == ClipType::Image;
}

bool ElementUtils::requiresMediaId(const Clip& clip) noexcept {
    return requiresMediaId(clip.type());
}

std::vector<ElementLocation> ElementUtils::getElementsAtTime(
    const Timeline& timeline,
    core::TimelineTime time,
    bool strictInterior
) {
    std::vector<ElementLocation> result;

    // Track traversal order: overlay -> main -> audio (matching web orderedTracks)
    for (const auto* track : timeline.allTracks()) {
        for (const auto& clip : track->clips()) {
            const auto start = clip.startTime();
            const auto end = clip.endTime();

            bool matches = false;
            if (strictInterior) {
                matches = (time > start && time < end);
            } else {
                matches = (time >= start && time < end);
            }

            if (matches) {
                result.push_back(ElementLocation{track->id(), clip.id()});
            }
        }
    }

    return result;
}

std::vector<std::string> ElementUtils::getElementFontFamilies(const Timeline& timeline) {
    std::set<std::string> uniqueFamilies;

    for (const auto* track : timeline.allTracks()) {
        for (const auto& clip : track->clips()) {
            if (clip.type() == ClipType::Text) {
                std::string font = clip.getParam<std::string>("fontFamily", "");
                if (!font.empty()) {
                    uniqueFamilies.insert(std::move(font));
                }
            }
        }
    }

    return std::vector<std::string>(uniqueFamilies.begin(), uniqueFamilies.end());
}

} // namespace catchim::editor
