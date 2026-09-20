#pragma once

#include "editor/subtitles/AssSubtitleParser.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <vector>

namespace catchim::editor {

class SubtitleElementBuilder {
public:
    static constexpr double SUBTITLE_MAX_WIDTH_RATIO = 0.8;
    static constexpr double SUBTITLE_BOTTOM_MARGIN_RATIO = 0.05;
    static constexpr double FONT_SIZE_SCALE_REFERENCE = 1080.0;

    static Clip buildSubtitleTextElement(
        size_t index,
        const AssSubtitleCue& cue,
        double canvasWidth = 1920.0,
        double canvasHeight = 1080.0
    );

    static std::string wrapSubtitleText(
        const std::string& text,
        double maxWidth,
        double approxCharWidth
    );

    static double resolveTargetWidth(
        double canvasWidth,
        const SubtitlePlacement& placement
    );

    static double resolvePositionX(
        double canvasWidth,
        SubtitleTextAlign textAlign,
        const SubtitlePlacement& placement,
        double approxTextWidth = 0.0
    );

    static double resolvePositionY(
        double canvasHeight,
        const SubtitlePlacement& placement,
        double approxTextHeight = 0.0
    );
};

} // namespace catchim::editor
