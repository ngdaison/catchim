#include "editor/panels/PanelLayoutManager.h"

namespace catchim::editor {

std::string panelIdToString(PanelId id) {
    switch (id) {
        case PanelId::Tools: return "tools";
        case PanelId::Preview: return "preview";
        case PanelId::Properties: return "properties";
        case PanelId::MainContent: return "mainContent";
        case PanelId::Timeline: return "timeline";
    }
    return "tools";
}

std::optional<PanelId> panelIdFromString(const std::string& str) {
    if (str == "tools" || str == "toolsPanel") return PanelId::Tools;
    if (str == "preview" || str == "previewPanel") return PanelId::Preview;
    if (str == "properties" || str == "propertiesPanel") return PanelId::Properties;
    if (str == "mainContent") return PanelId::MainContent;
    if (str == "timeline") return PanelId::Timeline;
    return std::nullopt;
}

double PanelSizes::get(PanelId id) const noexcept {
    switch (id) {
        case PanelId::Tools: return tools;
        case PanelId::Preview: return preview;
        case PanelId::Properties: return properties;
        case PanelId::MainContent: return mainContent;
        case PanelId::Timeline: return timeline;
    }
    return tools;
}

void PanelSizes::set(PanelId id, double size) noexcept {
    switch (id) {
        case PanelId::Tools: tools = size; break;
        case PanelId::Preview: preview = size; break;
        case PanelId::Properties: properties = size; break;
        case PanelId::MainContent: mainContent = size; break;
        case PanelId::Timeline: timeline = size; break;
    }
}

const PanelSizes& PanelLayoutManager::defaultSizes() noexcept {
    static const PanelSizes s_defaults{
        25.0, // tools
        50.0, // preview
        25.0, // properties
        50.0, // mainContent
        50.0  // timeline
    };
    return s_defaults;
}

PanelLayoutManager::PanelLayoutManager()
    : sizes_(defaultSizes()) {
}

PanelLayoutManager::PanelLayoutManager(PanelSizes initialSizes)
    : sizes_(initialSizes) {
}

void PanelLayoutManager::setPanel(PanelId id, double size) {
    sizes_.set(id, size);
    notifyListeners();
}

void PanelLayoutManager::setPanels(const PanelSizes& sizes) {
    sizes_ = sizes;
    notifyListeners();
}

void PanelLayoutManager::resetPanels() {
    sizes_ = defaultSizes();
    notifyListeners();
}

void PanelLayoutManager::addListener(LayoutChangeListener listener) {
    if (listener) {
        listeners_.push_back(std::move(listener));
    }
}

void PanelLayoutManager::clearListeners() {
    listeners_.clear();
}

void PanelLayoutManager::notifyListeners() {
    for (const auto& listener : listeners_) {
        if (listener) {
            listener(sizes_);
        }
    }
}

double PanelLayoutManager::clampPanelSize(double size, double minSize, double maxSize) noexcept {
    return std::clamp(size, minSize, maxSize);
}

nlohmann::json PanelLayoutManager::toJson() const {
    nlohmann::json j;
    j["version"] = 2;
    j["name"] = "panel-sizes";
    j["panels"] = {
        {"tools", sizes_.tools},
        {"preview", sizes_.preview},
        {"properties", sizes_.properties},
        {"mainContent", sizes_.mainContent},
        {"timeline", sizes_.timeline}
    };
    return j;
}

bool PanelLayoutManager::fromJson(const nlohmann::json& j) {
    try {
        sizes_ = migrateJson(j);
        notifyListeners();
        return true;
    } catch (...) {
        return false;
    }
}

PanelSizes PanelLayoutManager::migrateJson(const nlohmann::json& j) {
    const auto& def = defaultSizes();
    if (j.is_null() || !j.is_object()) {
        return def;
    }

    // Version 2 schema: { "panels": { ... } }
    if (j.contains("panels") && j["panels"].is_object()) {
        const auto& p = j["panels"];
        PanelSizes res = def;
        if (p.contains("tools") && p["tools"].is_number()) {
            res.tools = p["tools"].get<double>();
        }
        if (p.contains("preview") && p["preview"].is_number()) {
            res.preview = p["preview"].get<double>();
        }
        if (p.contains("properties") && p["properties"].is_number()) {
            res.properties = p["properties"].get<double>();
        }
        if (p.contains("mainContent") && p["mainContent"].is_number()) {
            res.mainContent = p["mainContent"].get<double>();
        }
        if (p.contains("timeline") && p["timeline"].is_number()) {
            res.timeline = p["timeline"].get<double>();
        }
        return res;
    }

    // Version 1 legacy schema: flattened keys (e.g. toolsPanel, previewPanel, etc.)
    PanelSizes res = def;
    if (j.contains("tools") && j["tools"].is_number()) {
        res.tools = j["tools"].get<double>();
    } else if (j.contains("toolsPanel") && j["toolsPanel"].is_number()) {
        res.tools = j["toolsPanel"].get<double>();
    }

    if (j.contains("preview") && j["preview"].is_number()) {
        res.preview = j["preview"].get<double>();
    } else if (j.contains("previewPanel") && j["previewPanel"].is_number()) {
        res.preview = j["previewPanel"].get<double>();
    }

    if (j.contains("properties") && j["properties"].is_number()) {
        res.properties = j["properties"].get<double>();
    } else if (j.contains("propertiesPanel") && j["propertiesPanel"].is_number()) {
        res.properties = j["propertiesPanel"].get<double>();
    }

    if (j.contains("mainContent") && j["mainContent"].is_number()) {
        res.mainContent = j["mainContent"].get<double>();
    }

    if (j.contains("timeline") && j["timeline"].is_number()) {
        res.timeline = j["timeline"].get<double>();
    }

    return res;
}

} // namespace catchim::editor
