#pragma once

#include "editor/project/Project.h"
#include "render/Compositor.h"
#include "core/time/TimelineTime.h"
#include <vector>
#include <string>
#include <cstdint>

namespace catchim::exporting {

class StillImageExporter {
public:
    static std::vector<uint8_t> renderFrameRgba(
        const editor::Project& project,
        core::TimelineTime time,
        int32_t width = 1920,
        int32_t height = 1080
    );

    static std::vector<uint8_t> encodeBmp(
        const uint8_t* rgba,
        int32_t w,
        int32_t h
    );

    static bool saveSnapshot(
        const editor::Project& project,
        core::TimelineTime time,
        const std::string& outputPath,
        int32_t w = 1920,
        int32_t h = 1080
    );
};

} // namespace catchim::exporting
