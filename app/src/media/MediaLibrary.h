#pragma once

#include "MediaAsset.h"
#include <vector>
#include <memory>
#include <optional>

namespace catchim::media {

class MediaLibrary {
public:
    MediaLibrary() = default;

    const std::vector<std::shared_ptr<MediaAsset>>& assets() const noexcept { return assets_; }
    std::vector<std::shared_ptr<MediaAsset>>& assets() noexcept { return assets_; }

    void addAsset(std::shared_ptr<MediaAsset> asset);
    bool removeAsset(const core::MediaId& id);

    std::shared_ptr<MediaAsset> findAsset(const core::MediaId& id) const;
    std::shared_ptr<MediaAsset> findAssetByPath(const std::filesystem::path& path) const;

    void clear();

private:
    std::vector<std::shared_ptr<MediaAsset>> assets_;
};

} // namespace catchim::media
