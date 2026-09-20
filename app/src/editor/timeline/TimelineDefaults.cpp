#include "editor/timeline/TimelineDefaults.h"

namespace catchim::editor {

core::TimelineTime TimelineDefaults::toElementDurationTicks(
    const std::optional<double>& seconds
) noexcept {
    if (!seconds.has_value() || *seconds <= 0.0) {
        return defaultNewElementDuration();
    }
    return core::TimelineTime::fromSeconds(*seconds);
}

nlohmann::json TimelineDefaults::buildDefaultTextElementParams() {
    auto textDefaults = defaultTextParams();
    auto transDefaults = defaultTransform();

    return nlohmann::json{
        {"content", textDefaults.content},
        {"fontSize", textDefaults.fontSize},
        {"fontFamily", textDefaults.fontFamily},
        {"color", textDefaults.color},
        {"textAlign", textDefaults.textAlign},
        {"fontWeight", textDefaults.fontWeight},
        {"fontStyle", textDefaults.fontStyle},
        {"textDecoration", textDefaults.textDecoration},
        {"letterSpacing", textDefaults.letterSpacing},
        {"lineHeight", textDefaults.lineHeight},
        {"background.enabled", textDefaults.background.enabled},
        {"background.color", textDefaults.background.color},
        {"background.cornerRadius", textDefaults.background.cornerRadius},
        {"background.paddingX", textDefaults.background.paddingX},
        {"background.paddingY", textDefaults.background.paddingY},
        {"background.offsetX", textDefaults.background.offsetX},
        {"background.offsetY", textDefaults.background.offsetY},
        {"transform.positionX", transDefaults.positionX},
        {"transform.positionY", transDefaults.positionY},
        {"transform.scaleX", transDefaults.scaleX},
        {"transform.scaleY", transDefaults.scaleY},
        {"transform.rotate", transDefaults.rotate},
        {"opacity", defaultOpacity()},
        {"blendMode", defaultBlendMode()}
    };
}

} // namespace catchim::editor
