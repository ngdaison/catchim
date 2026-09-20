#pragma once

namespace catchim::ui {

struct Metrics {
    static constexpr int headerHeight = 54;              // 3.4rem
    static constexpr int panelBorderRadius = 6;          // 0.35rem
    static constexpr int panelGap = 3;                   // 0.19rem

    // Timeline metrics
    static constexpr int trackHeightVideo = 65;
    static constexpr int trackHeightAudio = 50;
    static constexpr int trackHeightText = 25;
    static constexpr int trackHeightGraphic = 25;
    static constexpr int trackHeightEffect = 25;
    static constexpr int trackGap = 6;
    static constexpr int trackLabelsWidth = 112;
    static constexpr int timelineRulerHeight = 22;
    static constexpr int timelineBookmarkHeight = 16;
    static constexpr int timelineContentTopPadding = 2;
    static constexpr int timelineScrollbarSize = 12;
};

} // namespace catchim::ui
