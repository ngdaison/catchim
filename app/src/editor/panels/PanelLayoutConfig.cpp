#include "PanelLayoutConfig.h"

#include <algorithm>

namespace catchim::editor {

bool PanelLayoutConfig::isValidRatio(double ratio) noexcept {
    return ratio >= 0.0 && ratio <= 100.0;
}

double PanelLayoutConfig::clampPanelSize(double size, double minSize, double maxSize) noexcept {
    const double resolvedMin = std::max(0.0, minSize);
    const double resolvedMax = std::max(resolvedMin, maxSize);
    return std::max(resolvedMin, std::min(resolvedMax, size));
}

} // namespace catchim::editor
