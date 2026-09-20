#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <stdexcept>

namespace catchim::media {

struct ParsedStickerId {
    std::string providerId;
    std::string providerValue;

    bool operator==(const ParsedStickerId& other) const noexcept = default;
};

class StickerIdUtils {
public:
    static constexpr int STICKER_INTRINSIC_SIZE_FALLBACK = 200;

    static ParsedStickerId parseStickerId(std::string_view stickerId);
    static bool tryParseStickerId(std::string_view stickerId, ParsedStickerId& out) noexcept;
    static std::string buildStickerId(std::string_view providerId, std::string_view providerValue);

    static const std::vector<std::string>& getStickerCategories() noexcept;
};

} // namespace catchim::media
