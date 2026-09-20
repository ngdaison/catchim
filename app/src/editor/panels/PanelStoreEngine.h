#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

#include "PanelLayoutManager.h"

namespace catchim::editor {

class PanelStoreEngine {
public:
    static constexpr int CURRENT_VERSION = 2;
    static constexpr const char* STORAGE_KEY = "panel-sizes";

    PanelStoreEngine();
    explicit PanelStoreEngine(PanelSizes initialSizes);

    [[nodiscard]] const PanelSizes& getPanels() const noexcept { return panels_; }

    void setPanel(PanelId panel, double size) noexcept;
    void setPanel(const std::string& panelName, double size) noexcept;

    void setPanels(
        std::optional<double> tools = std::nullopt,
        std::optional<double> preview = std::nullopt,
        std::optional<double> properties = std::nullopt,
        std::optional<double> mainContent = std::nullopt,
        std::optional<double> timeline = std::nullopt
    ) noexcept;

    void resetPanels() noexcept;

    [[nodiscard]] nlohmann::json toJson() const;
    static PanelStoreEngine fromJson(const nlohmann::json& json);

    static std::string panelIdToString(PanelId id) noexcept;
    static std::optional<PanelId> panelIdFromString(std::string_view str) noexcept;

private:
    PanelSizes panels_;
};

} // namespace catchim::editor
