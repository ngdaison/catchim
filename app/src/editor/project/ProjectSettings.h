#pragma once

#include "core/time/TimelineTime.h"
#include <string>
#include <optional>

namespace catchim::editor {

struct CanvasSize {
    int32_t width{1920};
    int32_t height{1080};

    constexpr bool operator==(const CanvasSize& other) const noexcept = default;
};

struct ProjectBackground {
    enum class Type { Color, Blur };
    Type type{Type::Color};
    std::string color{"#000000"};
    double blurIntensity{10.0};

    bool operator==(const ProjectBackground& other) const = default;
};

struct ProjectSettings {
    core::FrameRate fps{30, 1};
    CanvasSize canvasSize{1920, 1080};
    std::string canvasSizeMode{"preset"}; // "preset" | "custom"
    std::optional<CanvasSize> lastCustomCanvasSize{std::nullopt};
    std::optional<CanvasSize> originalCanvasSize{std::nullopt};
    ProjectBackground background{};

    bool operator==(const ProjectSettings& other) const = default;
};

} // namespace catchim::editor
