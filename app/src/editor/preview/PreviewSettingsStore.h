#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <array>
#include <nlohmann/json.hpp>

namespace catchim::editor {

struct PreviewGridConfig {
    int rows{3};
    int cols{3};

    bool operator==(const PreviewGridConfig& other) const = default;
};

class PreviewSettingsStore {
public:
    static constexpr int STORE_VERSION = 6;
    static constexpr double PREVIEW_ZOOM_MIN = 0.25;
    static constexpr double PREVIEW_ZOOM_MAX = 16.0;
    static constexpr double PREVIEW_ZOOM_STEP = 1.25;
    static constexpr std::array<double, 6> PREVIEW_ZOOM_PRESETS = {0.25, 0.50, 0.75, 1.00, 1.50, 2.00};

    PreviewSettingsStore();

    const std::optional<std::string>& activeGuide() const noexcept { return activeGuide_; }
    void setActiveGuide(std::optional<std::string> guide) { activeGuide_ = std::move(guide); }
    void toggleGuide(const std::string& guideId);

    const PreviewGridConfig& gridConfig() const noexcept { return gridConfig_; }
    void setGridConfig(int rows, int cols) noexcept { gridConfig_.rows = rows; gridConfig_.cols = cols; }

    bool isOverlayVisible(const std::string& overlayId, bool defaultVisible = true) const;
    void setOverlayVisibility(const std::string& overlayId, bool isVisible);
    void toggleOverlayVisibility(const std::string& overlayId);

    nlohmann::json toJson() const;
    static PreviewSettingsStore fromJson(const nlohmann::json& j);

private:
    std::optional<std::string> activeGuide_{std::nullopt};
    std::unordered_map<std::string, bool> overlays_;
    PreviewGridConfig gridConfig_{3, 3};
};

} // namespace catchim::editor
