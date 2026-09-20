#include "MediaManager.h"
#include <algorithm>

namespace catchim::media {

const MediaAsset* MediaManager::findAsset(const core::MediaId& id) const noexcept {
    for (const auto& asset : assets_) {
        if (asset.id() == id) {
            return &asset;
        }
    }
    return nullptr;
}

MediaAsset* MediaManager::findAsset(const core::MediaId& id) noexcept {
    for (auto& asset : assets_) {
        if (asset.id() == id) {
            return &asset;
        }
    }
    return nullptr;
}

bool MediaManager::addMediaAsset(MediaAsset asset) {
    if (findAsset(asset.id()) != nullptr) {
        return false;
    }
    assets_.push_back(std::move(asset));
    notify();
    return true;
}

bool MediaManager::removeMediaAsset(const core::MediaId& id) {
    auto it = std::remove_if(assets_.begin(), assets_.end(), [&](const MediaAsset& a) {
        return a.id() == id;
    });

    if (it != assets_.end()) {
        assets_.erase(it, assets_.end());
        notify();
        return true;
    }
    return false;
}

size_t MediaManager::removeMediaAssets(const std::vector<core::MediaId>& ids) {
    if (ids.empty()) return 0;

    size_t removedCount = 0;
    for (const auto& id : ids) {
        if (removeMediaAsset(id)) {
            ++removedCount;
        }
    }
    return removedCount;
}

void MediaManager::clearAllAssets() noexcept {
    if (!assets_.empty()) {
        assets_.clear();
        notify();
    }
}

void MediaManager::setAssets(std::vector<MediaAsset> assets) {
    assets_ = std::move(assets);
    notify();
}

void MediaManager::setIsLoading(bool loading) noexcept {
    if (isLoading_ != loading) {
        isLoading_ = loading;
        notify();
    }
}

void MediaManager::subscribe(MediaChangeListener listener) {
    listeners_.push_back(std::move(listener));
}

void MediaManager::notify() {
    for (const auto& l : listeners_) {
        if (l) l();
    }
}

} // namespace catchim::media
