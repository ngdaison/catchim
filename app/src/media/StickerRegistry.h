#pragma once

#include "core/time/TimelineTime.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace catchim::media {

struct StickerItem {
    std::string id;
    std::string name;
    std::string category;
    int intrinsicWidth{256};
    int intrinsicHeight{256};
    std::string assetPath;

    double aspectRatio() const noexcept {
        return intrinsicHeight > 0 ? static_cast<double>(intrinsicWidth) / intrinsicHeight : 1.0;
    }
};

class StickerRegistry {
public:
    static StickerRegistry& instance();

    StickerRegistry();

    void registerSticker(StickerItem item);
    std::vector<std::string> getCategories() const;
    std::vector<StickerItem> getStickersByCategory(const std::string& category) const;
    const StickerItem* findSticker(const std::string& id) const;

    editor::Clip createStickerClip(
        const std::string& stickerId,
        core::TimelineTime startTime,
        core::TimelineTime duration = core::TimelineTime::fromSeconds(3.0)
    ) const;

    size_t totalCount() const { return stickers_.size(); }

private:
    void initDefaultPresets();

    std::unordered_map<std::string, StickerItem> stickers_;
    std::vector<std::string> categories_;
};

} // namespace catchim::media
