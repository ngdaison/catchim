#include "MediaLibrary.h"
#include <algorithm>

namespace catchim::media {

void MediaLibrary::addAsset(std::shared_ptr<MediaAsset> asset) {
    if (!asset) return;
    auto it = std::find_if(assets_.begin(), assets_.end(), [&](const auto& a) {
        return a->id() == asset->id();
    });
    if (it == assets_.end()) {
        assets_.push_back(std::move(asset));
    }
}

bool MediaLibrary::removeAsset(const core::MediaId& id) {
    auto it = std::find_if(assets_.begin(), assets_.end(), [&](const auto& a) {
        return a->id() == id;
    });
    if (it != assets_.end()) {
        assets_.erase(it);
        return true;
    }
    return false;
}

std::shared_ptr<MediaAsset> MediaLibrary::findAsset(const core::MediaId& id) const {
    for (const auto& a : assets_) {
        if (a->id() == id) return a;
    }
    return nullptr;
}

std::shared_ptr<MediaAsset> MediaLibrary::findAssetByPath(const std::filesystem::path& path) const {
    for (const auto& a : assets_) {
        if (std::filesystem::equivalent(a->filePath(), path)) return a;
    }
    return nullptr;
}

void MediaLibrary::clear() {
    assets_.clear();
}

} // namespace catchim::media
