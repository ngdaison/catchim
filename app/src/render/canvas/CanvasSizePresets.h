#pragma once

#include "editor/project/ProjectSettings.h"
#include <vector>

namespace catchim::render {

class CanvasSizePresets {
public:
    static inline constexpr editor::CanvasSize DEFAULT_CANVAS_SIZE{1920, 1080};

    static const std::vector<editor::CanvasSize>& defaultCanvasPresets() noexcept;

    static bool isDefaultPreset(const editor::CanvasSize& size) noexcept;

    static double getAspectRatio(const editor::CanvasSize& size) noexcept;
};

} // namespace catchim::render
