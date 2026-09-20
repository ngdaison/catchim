#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <functional>
#include <vector>
#include <algorithm>

namespace catchim::editor {

enum class PanelId {
    Tools,
    Preview,
    Properties,
    MainContent,
    Timeline
};

std::string panelIdToString(PanelId id);
std::optional<PanelId> panelIdFromString(const std::string& str);

struct PanelSizes {
    double tools{25.0};
    double preview{50.0};
    double properties{25.0};
    double mainContent{50.0};
    double timeline{50.0};

    bool operator==(const PanelSizes& other) const noexcept {
        return tools == other.tools &&
               preview == other.preview &&
               properties == other.properties &&
               mainContent == other.mainContent &&
               timeline == other.timeline;
    }

    bool operator!=(const PanelSizes& other) const noexcept {
        return !(*this == other);
    }

    double get(PanelId id) const noexcept;
    void set(PanelId id, double size) noexcept;
};

/**
 * @brief Manages editor panel layout sizes, persistence, and migrations.
 * Corresponds to web/src/editor/panel-store.ts and web/src/panels/layout.ts.
 */
class PanelLayoutManager {
public:
    using LayoutChangeListener = std::function<void(const PanelSizes&)>;

    PanelLayoutManager();
    explicit PanelLayoutManager(PanelSizes initialSizes);

    static const PanelSizes& defaultSizes() noexcept;

    const PanelSizes& sizes() const noexcept { return sizes_; }
    double getPanel(PanelId id) const noexcept { return sizes_.get(id); }

    void setPanel(PanelId id, double size);
    void setPanels(const PanelSizes& sizes);
    void resetPanels();

    void addListener(LayoutChangeListener listener);
    void clearListeners();

    nlohmann::json toJson() const;
    bool fromJson(const nlohmann::json& j);

    static PanelSizes migrateJson(const nlohmann::json& j);
    static double clampPanelSize(double size, double minSize = 5.0, double maxSize = 95.0) noexcept;

private:
    void notifyListeners();

    PanelSizes sizes_;
    std::vector<LayoutChangeListener> listeners_;
};

} // namespace catchim::editor
