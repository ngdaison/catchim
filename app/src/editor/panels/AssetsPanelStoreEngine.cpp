#include "AssetsPanelStoreEngine.h"

namespace catchim::editor {

const char* AssetsPanelStoreEngine::tabToString(AssetsTab tab) noexcept {
    switch (tab) {
        case AssetsTab::Media: return "media";
        case AssetsTab::Sounds: return "sounds";
        case AssetsTab::Text: return "text";
        case AssetsTab::Stickers: return "stickers";
        case AssetsTab::Effects: return "effects";
        case AssetsTab::Transitions: return "transitions";
        case AssetsTab::Captions: return "captions";
        case AssetsTab::Adjustment: return "adjustment";
        case AssetsTab::Settings: return "settings";
    }
    return "media";
}

AssetsTab AssetsPanelStoreEngine::stringToTab(std::string_view sv) noexcept {
    if (sv == "sounds") return AssetsTab::Sounds;
    if (sv == "text") return AssetsTab::Text;
    if (sv == "stickers") return AssetsTab::Stickers;
    if (sv == "effects") return AssetsTab::Effects;
    if (sv == "transitions") return AssetsTab::Transitions;
    if (sv == "captions") return AssetsTab::Captions;
    if (sv == "adjustment") return AssetsTab::Adjustment;
    if (sv == "settings") return AssetsTab::Settings;
    return AssetsTab::Media;
}

const char* AssetsPanelStoreEngine::viewModeToString(MediaViewMode mode) noexcept {
    switch (mode) {
        case MediaViewMode::Grid: return "grid";
        case MediaViewMode::List: return "list";
    }
    return "grid";
}

MediaViewMode AssetsPanelStoreEngine::stringToViewMode(std::string_view sv) noexcept {
    if (sv == "list") return MediaViewMode::List;
    return MediaViewMode::Grid;
}

const char* AssetsPanelStoreEngine::sortKeyToString(MediaSortKey key) noexcept {
    switch (key) {
        case MediaSortKey::Name: return "name";
        case MediaSortKey::Type: return "type";
        case MediaSortKey::Duration: return "duration";
        case MediaSortKey::Size: return "size";
    }
    return "name";
}

MediaSortKey AssetsPanelStoreEngine::stringToSortKey(std::string_view sv) noexcept {
    if (sv == "type") return MediaSortKey::Type;
    if (sv == "duration") return MediaSortKey::Duration;
    if (sv == "size") return MediaSortKey::Size;
    return MediaSortKey::Name;
}

const char* AssetsPanelStoreEngine::sortOrderToString(MediaSortOrder order) noexcept {
    switch (order) {
        case MediaSortOrder::Asc: return "asc";
        case MediaSortOrder::Desc: return "desc";
    }
    return "asc";
}

MediaSortOrder AssetsPanelStoreEngine::stringToSortOrder(std::string_view sv) noexcept {
    if (sv == "desc") return MediaSortOrder::Desc;
    return MediaSortOrder::Asc;
}

void AssetsPanelStoreEngine::requestRevealMedia(const std::string& mediaId) {
    activeTab_ = AssetsTab::Media;
    highlightMediaId_ = mediaId;
}

nlohmann::json AssetsPanelStoreEngine::toJson() const {
    nlohmann::json j = nlohmann::json::object();
    j["activeTab"] = tabToString(activeTab_);
    j["mediaViewMode"] = viewModeToString(mediaViewMode_);
    j["mediaSortBy"] = sortKeyToString(mediaSortBy_);
    j["mediaSortOrder"] = sortOrderToString(mediaSortOrder_);
    if (highlightMediaId_.has_value()) {
        j["highlightMediaId"] = *highlightMediaId_;
    }
    return j;
}

void AssetsPanelStoreEngine::fromJson(const nlohmann::json& j) {
    if (!j.is_object()) return;

    if (j.contains("activeTab") && j["activeTab"].is_string()) {
        activeTab_ = stringToTab(j["activeTab"].get<std::string>());
    }
    if (j.contains("mediaViewMode") && j["mediaViewMode"].is_string()) {
        mediaViewMode_ = stringToViewMode(j["mediaViewMode"].get<std::string>());
    }
    if (j.contains("mediaSortBy") && j["mediaSortBy"].is_string()) {
        mediaSortBy_ = stringToSortKey(j["mediaSortBy"].get<std::string>());
    }
    if (j.contains("mediaSortOrder") && j["mediaSortOrder"].is_string()) {
        mediaSortOrder_ = stringToSortOrder(j["mediaSortOrder"].get<std::string>());
    }
    if (j.contains("highlightMediaId") && j["highlightMediaId"].is_string()) {
        highlightMediaId_ = j["highlightMediaId"].get<std::string>();
    } else {
        highlightMediaId_.reset();
    }
}

} // namespace catchim::editor
