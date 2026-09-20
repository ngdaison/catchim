#pragma once

#include "editor/project/ProjectSettings.h"
#include <string>

namespace catchim::exporting {

enum class ExportFormat {
    MP4,
    WebM,
    FrameSequence
};

enum class ExportQuality {
    Low,
    Medium,
    High,
    VeryHigh
};

struct ExportSettings {
    editor::CanvasSize resolution{1920, 1080};
    core::FrameRate fps{30, 1};
    ExportFormat format{ExportFormat::MP4};
    ExportQuality quality{ExportQuality::High};
    bool includeAudio{true};
    int audioSampleRate{44100};
    int audioChannels{2};
    std::string outputPath;

    // Bitrates (bps)
    int64_t videoBitrate() const {
        switch (quality) {
            case ExportQuality::Low: return 2'500'000;
            case ExportQuality::Medium: return 6'000'000;
            case ExportQuality::High: return 12'000'000;
            case ExportQuality::VeryHigh: return 25'000'000;
        }
        return 10'000'000;
    }

    int audioBitrate() const {
        return (quality == ExportQuality::Low) ? 128'000 : 320'000;
    }
};

struct ExportResult {
    bool success{false};
    std::string outputPath;
    std::string error;
    bool isCancelled{false};
    int64_t totalFrames{0};
    double durationSeconds{0.0};
};

} // namespace catchim::exporting
