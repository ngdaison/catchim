#include "media/StickerRegistry.h"
#include <algorithm>

namespace catchim::media {

StickerRegistry& StickerRegistry::instance() {
    static StickerRegistry s_instance;
    return s_instance;
}

StickerRegistry::StickerRegistry() {
    initDefaultPresets();
}

void StickerRegistry::initDefaultPresets() {
    registerSticker({"arrow-right", "Mũi tên phải", "arrows", 256, 128, "stickers/arrows/right.svg"});
    registerSticker({"arrow-curved", "Mũi tên cong", "arrows", 256, 256, "stickers/arrows/curved.svg"});
    registerSticker({"arrow-pointer", "Con trỏ chỉ", "arrows", 256, 256, "stickers/arrows/pointer.svg"});

    registerSticker({"badge-star", "Huy hiệu sao", "badges", 256, 256, "stickers/badges/star.svg"});
    registerSticker({"badge-verified", "Tích xanh", "badges", 256, 256, "stickers/badges/verified.svg"});
    registerSticker({"badge-discount", "Nhãn giảm giá", "badges", 256, 256, "stickers/badges/discount.svg"});

    registerSticker({"emoji-smile", "Mặt cười", "emojis", 256, 256, "stickers/emojis/smile.svg"});
    registerSticker({"emoji-fire", "Ngọn lửa", "emojis", 256, 256, "stickers/emojis/fire.svg"});
    registerSticker({"emoji-heart", "Trái tim", "emojis", 256, 256, "stickers/emojis/heart.svg"});
    registerSticker({"emoji-thumbsup", "Thích", "emojis", 256, 256, "stickers/emojis/thumbsup.svg"});

    registerSticker({"shape-circle", "Hình tròn", "shapes", 256, 256, "stickers/shapes/circle.svg"});
    registerSticker({"shape-star", "Ngôi sao 5 cánh", "shapes", 256, 256, "stickers/shapes/star.svg"});
    registerSticker({"shape-rect", "Hình chữ nhật", "shapes", 300, 150, "stickers/shapes/rect.svg"});

    registerSticker({"social-like", "Nút Thích", "social", 256, 100, "stickers/social/like.svg"});
    registerSticker({"social-share", "Nút Chia sẻ", "social", 256, 100, "stickers/social/share.svg"});
    registerSticker({"social-subscribe", "Nút Đăng ký", "social", 320, 100, "stickers/social/subscribe.svg"});
}

void StickerRegistry::registerSticker(StickerItem item) {
    if (std::find(categories_.begin(), categories_.end(), item.category) == categories_.end()) {
        categories_.push_back(item.category);
    }
    stickers_[item.id] = std::move(item);
}

std::vector<std::string> StickerRegistry::getCategories() const {
    return categories_;
}

std::vector<StickerItem> StickerRegistry::getStickersByCategory(const std::string& category) const {
    std::vector<StickerItem> result;
    for (const auto& [_, item] : stickers_) {
        if (item.category == category) {
            result.push_back(item);
        }
    }
    return result;
}

const StickerItem* StickerRegistry::findSticker(const std::string& id) const {
    auto it = stickers_.find(id);
    if (it != stickers_.end()) {
        return &it->second;
    }
    return nullptr;
}

editor::Clip StickerRegistry::createStickerClip(
    const std::string& stickerId,
    core::TimelineTime startTime,
    core::TimelineTime duration
) const {
    const auto* sticker = findSticker(stickerId);
    std::string name = sticker ? sticker->name : "Sticker";

    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Graphic,
        name,
        startTime,
        duration
    );

    clip.setParam("stickerId", stickerId);
    if (sticker) {
        clip.setParam("category", sticker->category);
        clip.setParam("intrinsicWidth", sticker->intrinsicWidth);
        clip.setParam("intrinsicHeight", sticker->intrinsicHeight);
    }
    clip.setParam("transform.scaleX", 1.0);
    clip.setParam("transform.scaleY", 1.0);

    return clip;
}

} // namespace catchim::media
