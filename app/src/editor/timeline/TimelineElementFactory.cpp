#include "TimelineElementFactory.h"

namespace catchim::editor {

Clip TimelineElementFactory::buildGraphicElement(
    const core::ClipId& clipId,
    const std::string& graphicType,
    core::TimelineTime startTime,
    core::TimelineTime duration
) {
    Clip clip(clipId, ClipType::Graphic, "Graphic (" + graphicType + ")", startTime, duration);
    return clip;
}

Clip TimelineElementFactory::buildStickerElement(
    const core::ClipId& clipId,
    const std::string& stickerId,
    core::TimelineTime startTime,
    core::TimelineTime duration
) {
    Clip clip(clipId, ClipType::Sticker, "Sticker (" + stickerId + ")", startTime, duration);
    return clip;
}

Clip TimelineElementFactory::buildEffectElement(
    const core::ClipId& clipId,
    const std::string& effectType,
    core::TimelineTime startTime,
    core::TimelineTime duration
) {
    Clip clip(clipId, ClipType::Effect, "Effect (" + effectType + ")", startTime, duration);
    return clip;
}

bool TimelineElementFactory::canElementHaveAudio(ClipType type) noexcept {
    return type == ClipType::Audio || type == ClipType::Video;
}

bool TimelineElementFactory::isVisualElement(ClipType type) noexcept {
    return type == ClipType::Video ||
           type == ClipType::Image ||
           type == ClipType::Text ||
           type == ClipType::Graphic ||
           type == ClipType::Sticker;
}

bool TimelineElementFactory::isMaskableElement(ClipType type) noexcept {
    return type == ClipType::Video ||
           type == ClipType::Image ||
           type == ClipType::Graphic ||
           type == ClipType::Sticker;
}

bool TimelineElementFactory::isRetimableElement(ClipType type) noexcept {
    return type == ClipType::Video || type == ClipType::Audio;
}

bool TimelineElementFactory::canElementBeHidden(ClipType type) noexcept {
    return isVisualElement(type);
}

bool TimelineElementFactory::requiresMediaId(ClipType type) noexcept {
    return type == ClipType::Video ||
           type == ClipType::Image ||
           type == ClipType::Audio;
}

} // namespace catchim::editor
