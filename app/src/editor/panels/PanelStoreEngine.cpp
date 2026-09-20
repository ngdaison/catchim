#include "PanelStoreEngine.h"
#include <algorithm>

namespace catchim::editor {

PanelStoreEngine::PanelStoreEngine()
    : panels_{25.0, 50.0, 25.0, 50.0, 50.0} {}

PanelStoreEngine::PanelStoreEngine(PanelSizes initialSizes)
    : panels_(initialSizes) {}

void PanelStoreEngine::setPanel(PanelId panel, double size) noexcept {
    switch (panel) {
        case PanelId::Tools:
            panels_.tools = size;
            break;
        case PanelId::Preview:
            panels_.preview = size;
            break;
        case PanelId::Properties:
            panels_.properties = size;
            break;
        case PanelId::MainContent:
            panels_.mainContent = size;
            break;
        case PanelId::Timeline:
            panels_.timeline = size;
            break;
    }
}

void PanelStoreEngine::setPanel(const std::string& panelName, double size) noexcept {
    auto idOpt = panelIdFromString(panelName);
    if (idOpt.has_value()) {
        setPanel(*idOpt, size);
    }
}

void PanelStoreEngine::setPanels(
    std::optional<double> tools,
    std::optional<double> preview,
    std::optional<double> properties,
    std::optional<double> mainContent,
    std::optional<double> timeline
) noexcept {
    if (tools.has_value()) panels_.tools = *tools;
    if (preview.has_value()) panels_.preview = *preview;
    if (properties.has_value()) panels_.properties = *properties;
    if (mainContent.has_value()) panels_.mainContent = *mainContent;
    if (timeline.has_value()) panels_.timeline = *timeline;
}

void PanelStoreEngine::resetPanels() noexcept {
    panels_ = PanelSizes{25.0, 50.0, 25.0, 50.0, 50.0};
}

nlohmann::json PanelStoreEngine::toJson() const {
    nlohmann::json j;
    j["version"] = CURRENT_VERSION;
    j["panels"] = {
        {"tools", panels_.tools},
        {"preview", panels_.preview},
        {"properties", panels_.properties},
        {"mainContent", panels_.mainContent},
        {"timeline", panels_.timeline}
    };
    return j;
}

PanelStoreEngine PanelStoreEngine::fromJson(const nlohmann::json& json) {
    PanelStoreEngine store;
    if (!json.is_object()) return store;

    // Check if nested "panels" exists (v2 format)
    if (json.contains("panels") && json["panels"].is_object()) {
        const auto& p = json["panels"];
        if (p.contains("tools") && p["tools"].is_number()) {
            store.panels_.tools = p["tools"].get<double>();
        }
        if (p.contains("preview") && p["preview"].is_number()) {
            store.panels_.preview = p["preview"].get<double>();
        }
        if (p.contains("properties") && p["properties"].is_number()) {
            store.panels_.properties = p["properties"].get<double>();
        }
        if (p.contains("mainContent") && p["mainContent"].is_number()) {
            store.panels_.mainContent = p["mainContent"].get<double>();
        }
        if (p.contains("timeline") && p["timeline"].is_number()) {
            store.panels_.timeline = p["timeline"].get<double>();
        }
        return store;
    }

    // Migration from v1 or flat fields
    double tools = 25.0;
    if (json.contains("tools") && json["tools"].is_number()) {
        tools = json["tools"].get<double>();
    } else if (json.contains("toolsPanel") && json["toolsPanel"].is_number()) {
        tools = json["toolsPanel"].get<double>();
    }

    double preview = 50.0;
    if (json.contains("preview") && json["preview"].is_number()) {
        preview = json["preview"].get<double>();
    } else if (json.contains("previewPanel") && json["previewPanel"].is_number()) {
        preview = json["previewPanel"].get<double>();
    }

    double properties = 25.0;
    if (json.contains("properties") && json["properties"].is_number()) {
        properties = json["properties"].get<double>();
    } else if (json.contains("propertiesPanel") && json["propertiesPanel"].is_number()) {
        properties = json["propertiesPanel"].get<double>();
    }

    double mainContent = 50.0;
    if (json.contains("mainContent") && json["mainContent"].is_number()) {
        mainContent = json["mainContent"].get<double>();
    }

    double timeline = 50.0;
    if (json.contains("timeline") && json["timeline"].is_number()) {
        timeline = json["timeline"].get<double>();
    }

    store.panels_ = PanelSizes{tools, preview, properties, mainContent, timeline};
    return store;
}

std::string PanelStoreEngine::panelIdToString(PanelId id) noexcept {
    switch (id) {
        case PanelId::Tools: return "tools";
        case PanelId::Preview: return "preview";
        case PanelId::Properties: return "properties";
        case PanelId::MainContent: return "mainContent";
        case PanelId::Timeline: return "timeline";
    }
    return "unknown";
}

std::optional<PanelId> PanelStoreEngine::panelIdFromString(std::string_view str) noexcept {
    if (str == "tools") return PanelId::Tools;
    if (str == "preview") return PanelId::Preview;
    if (str == "properties") return PanelId::Properties;
    if (str == "mainContent") return PanelId::MainContent;
    if (str == "timeline") return PanelId::Timeline;
    return std::nullopt;
}

} // namespace catchim::editor
