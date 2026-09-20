#pragma once

#include "MediaAsset.h"
#include "core/ids/Ids.h"
#include <vector>
#include <string>
#include <functional>

namespace catchim::media {

class MediaManager {
public:
    using MediaChangeListener = std::function<void()>;

    MediaManager() = default;

    const std::vector<MediaAsset>& getAssets() const noexcept { return assets_; }
    std::vector<MediaAsset>& getAssets() noexcept { return assets_; }

    const MediaAsset* findAsset(const core::MediaId& id) const noexcept;
    MediaAsset* findAsset(const core::MediaId& id) noexcept;

    bool addMediaAsset(MediaAsset asset);
    bool removeMediaAsset(const core::MediaId& id);
    size_t removeMediaAssets(const std::vector<core::MediaId>& ids);

    void clearAllAssets() noexcept;
    void setAssets(std::vector<MediaAsset> assets);

    bool isLoadingMedia() const noexcept { return isLoading_; }
    void setIsLoading(bool loading) noexcept;

    void subscribe(MediaChangeListener listener);

private:
    void notify();

    std::vector<MediaAsset> assets_;
    bool isLoading_{false};
    std::vector<MediaChangeListener> listeners_;
};

} // namespace catchim::media
