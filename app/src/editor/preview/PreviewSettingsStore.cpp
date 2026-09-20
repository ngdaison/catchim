#include "PreviewSettingsStore.h"

namespace catchim::editor {

PreviewSettingsStore::PreviewSettingsStore() = default;

void PreviewSettingsStore::toggleGuide(const std::string& guideId) {
    if (activeGuide_.has_value() && *activeGuide_ == guideId) {
        activeGuide_ = std::nullopt;
    } else {
        activeGuide_ = guideId;
    }
}

bool PreviewSettingsStore::isOverlayVisible(const std::string& overlayId, bool defaultVisible) const {
    auto it = overlays_.find(overlayId);
    if (it != overlays_.end()) {
        return it->second;
    }
    return defaultVisible;
}

void PreviewSettingsStore::setOverlayVisibility(const std::string& overlayId, bool isVisible) {
    overlays_[overlayId] = isVisible;
}

void PreviewSettingsStore::toggleOverlayVisibility(const std::string& overlayId) {
    overlays_[overlayId] = !isOverlayVisible(overlayId, true);
}

nlohmann::json PreviewSettingsStore::toJson() const {
    nlohmann::json j;
    j["version"] = STORE_VERSION;
    if (activeGuide_.has_value()) {
        j["activeGuide"] = *activeGuide_;
    } else {
        j["activeGuide"] = nullptr;
    }

    nlohmann::json ov = nlohmann::json::object();
    for (const auto& [k, v] : overlays_) {
        ov[k] = v;
    }
    j["overlays"] = ov;

    j["gridConfig"] = {
        {"rows", gridConfig_.rows},
        {"cols", gridConfig_.cols}
    };
    return j;
}

PreviewSettingsStore PreviewSettingsStore::fromJson(const nlohmann::json& j) {
    PreviewSettingsStore store;
    if (!j.is_object()) return store;

    if (j.contains("activeGuide") && j["activeGuide"].is_string()) {
        store.activeGuide_ = j["activeGuide"].get<std::string>();
    }

    if (j.contains("overlays") && j["overlays"].is_object()) {
        for (auto it = j["overlays"].begin(); it != j["overlays"].end(); ++it) {
            if (it.value().is_boolean()) {
                store.overlays_[it.key()] = it.value().get<bool>();
            }
        }
    }

    if (j.contains("gridConfig") && j["gridConfig"].is_object()) {
        const auto& gc = j["gridConfig"];
        if (gc.contains("rows") && gc["rows"].is_number_integer()) {
            store.gridConfig_.rows = gc["rows"].get<int>();
        }
        if (gc.contains("cols") && gc["cols"].is_number_integer()) {
            store.gridConfig_.cols = gc["cols"].get<int>();
        }
    }

    return store;
}

} // namespace catchim::editor
