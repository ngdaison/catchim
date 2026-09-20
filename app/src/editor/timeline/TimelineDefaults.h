#pragma once

#include "core/time/TimelineTime.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>

namespace catchim::editor {

struct DefaultTransform {
    double scaleX{1.0};
    double scaleY{1.0};
    double positionX{0.0};
    double positionY{0.0};
    double rotate{0.0};
};

struct DefaultTextBackground {
    bool enabled{false};
    std::string color{"#000000"};
    double cornerRadius{0.0};
    double paddingX{30.0};
    double paddingY{42.0};
    double offsetX{0.0};
    double offsetY{0.0};
};

struct DefaultTextParams {
    std::string content{"Default text"};
    double fontSize{15.0};
    std::string fontFamily{"Arial"};
    std::string color{"#ffffff"};
    std::string textAlign{"center"};
    std::string fontWeight{"normal"};
    std::string fontStyle{"normal"};
    std::string textDecoration{"none"};
    double letterSpacing{0.0};
    double lineHeight{1.2};
    DefaultTextBackground background;
};

struct DefaultTimelineViewState {
    double zoomLevel{1.0};
    double scrollLeft{0.0};
    core::TimelineTime playheadTime{0};
};

/**
 * @brief Default values for timeline elements, transforms, text parameters, and view state.
 * Corresponds to web/src/timeline/defaults.ts and web/src/timeline/creation.ts.
 */
class TimelineDefaults {
public:
    static constexpr int64_t DEFAULT_NEW_ELEMENT_DURATION_SECONDS = 5;

    static core::TimelineTime defaultNewElementDuration() noexcept {
        return core::TimelineTime::fromSeconds(DEFAULT_NEW_ELEMENT_DURATION_SECONDS);
    }

    static core::TimelineTime toElementDurationTicks(
        const std::optional<double>& seconds
    ) noexcept;

    static DefaultTransform defaultTransform() noexcept {
        return DefaultTransform{};
    }

    static double defaultOpacity() noexcept { return 1.0; }
    static std::string defaultBlendMode() noexcept { return "normal"; }
    static double defaultVolume() noexcept { return 0.0; }

    static DefaultTextParams defaultTextParams() noexcept {
        return DefaultTextParams{};
    }

    static nlohmann::json buildDefaultTextElementParams();

    static DefaultTimelineViewState defaultTimelineViewState() noexcept {
        return DefaultTimelineViewState{};
    }
};

} // namespace catchim::editor
