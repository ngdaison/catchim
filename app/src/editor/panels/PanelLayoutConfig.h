#pragma once

namespace catchim::editor {

struct PanelRatios {
    double tools{25.0};
    double preview{50.0};
    double properties{25.0};
    double mainContent{50.0};
    double timeline{50.0};

    constexpr bool operator==(const PanelRatios& other) const noexcept = default;
};

class PanelLayoutConfig {
public:
    static constexpr PanelRatios DEFAULT_CONFIG = {25.0, 50.0, 25.0, 50.0, 50.0};

    static PanelRatios getDefaultRatios() noexcept { return DEFAULT_CONFIG; }
    static bool isValidRatio(double ratio) noexcept;
    static double clampPanelSize(double size, double minSize, double maxSize) noexcept;
};

} // namespace catchim::editor
