#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::editor {

enum class AssetsTab {
    Media,
    Sounds,
    Text,
    Stickers,
    Effects,
    Transitions,
    Captions,
    Adjustment,
    Settings
};

enum class MediaViewMode {
    Grid,
    List
};

enum class MediaSortKey {
    Name,
    Type,
    Duration,
    Size
};

enum class MediaSortOrder {
    Asc,
    Desc
};

class AssetsPanelStoreEngine {
public:
    AssetsPanelStoreEngine() = default;

    static const char* tabToString(AssetsTab tab) noexcept;
    static AssetsTab stringToTab(std::string_view sv) noexcept;

    static const char* viewModeToString(MediaViewMode mode) noexcept;
    static MediaViewMode stringToViewMode(std::string_view sv) noexcept;

    static const char* sortKeyToString(MediaSortKey key) noexcept;
    static MediaSortKey stringToSortKey(std::string_view sv) noexcept;

    static const char* sortOrderToString(MediaSortOrder order) noexcept;
    static MediaSortOrder stringToSortOrder(std::string_view sv) noexcept;

    AssetsTab activeTab() const noexcept { return activeTab_; }
    void setActiveTab(AssetsTab tab) noexcept { activeTab_ = tab; }

    const std::optional<std::string>& highlightMediaId() const noexcept { return highlightMediaId_; }
    void requestRevealMedia(const std::string& mediaId);
    void clearHighlight() noexcept { highlightMediaId_.reset(); }

    MediaViewMode mediaViewMode() const noexcept { return mediaViewMode_; }
    void setMediaViewMode(MediaViewMode mode) noexcept { mediaViewMode_ = mode; }

    MediaSortKey mediaSortBy() const noexcept { return mediaSortBy_; }
    MediaSortOrder mediaSortOrder() const noexcept { return mediaSortOrder_; }
    void setMediaSort(MediaSortKey key, MediaSortOrder order) noexcept {
        mediaSortBy_ = key;
        mediaSortOrder_ = order;
    }

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);

private:
    AssetsTab activeTab_{AssetsTab::Media};
    std::optional<std::string> highlightMediaId_{std::nullopt};
    MediaViewMode mediaViewMode_{MediaViewMode::Grid};
    MediaSortKey mediaSortBy_{MediaSortKey::Name};
    MediaSortOrder mediaSortOrder_{MediaSortOrder::Asc};
};

} // namespace catchim::editor
