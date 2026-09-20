#include "PropertiesPanelStoreEngine.h"

namespace catchim::editor {

std::string PropertiesPanelStoreEngine::getActiveTab(
    const std::string& elementType,
    const std::string& defaultTab
) const {
    auto it = activeTabPerType_.find(elementType);
    if (it != activeTabPerType_.end()) {
        return it->second;
    }
    return defaultTab;
}

void PropertiesPanelStoreEngine::setActiveTab(
    const std::string& elementType,
    const std::string& tabId
) {
    activeTabPerType_[elementType] = tabId;
}

nlohmann::json PropertiesPanelStoreEngine::toJson() const {
    nlohmann::json j = nlohmann::json::object();
    j["activeTabPerType"] = activeTabPerType_;
    j["isTransformScaleLocked"] = isTransformScaleLocked_;
    return j;
}

void PropertiesPanelStoreEngine::fromJson(const nlohmann::json& j) {
    if (!j.is_object()) return;

    if (j.contains("activeTabPerType") && j["activeTabPerType"].is_object()) {
        activeTabPerType_.clear();
        for (const auto& [k, v] : j["activeTabPerType"].items()) {
            if (v.is_string()) {
                activeTabPerType_[k] = v.get<std::string>();
            }
        }
    }

    if (j.contains("isTransformScaleLocked") && j["isTransformScaleLocked"].is_boolean()) {
        isTransformScaleLocked_ = j["isTransformScaleLocked"].get<bool>();
    }
}

} // namespace catchim::editor
