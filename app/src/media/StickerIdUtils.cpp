#include "StickerIdUtils.h"
#include <cctype>

namespace catchim::media {

namespace {
std::string trim(std::string_view sv) {
    size_t start = 0;
    while (start < sv.size() && std::isspace(static_cast<unsigned char>(sv[start]))) {
        ++start;
    }
    size_t end = sv.size();
    while (end > start && std::isspace(static_cast<unsigned char>(sv[end - 1]))) {
        --end;
    }
    return std::string(sv.substr(start, end - start));
}
} // namespace

ParsedStickerId StickerIdUtils::parseStickerId(std::string_view stickerId) {
    std::string trimmed = trim(stickerId);
    if (trimmed.empty()) {
        throw std::invalid_argument("Sticker ID must be a non-empty string");
    }

    size_t sep = trimmed.find(':');
    if (sep == std::string::npos || sep == 0 || sep == trimmed.size() - 1) {
        throw std::invalid_argument("Invalid sticker ID format: \"" + std::string(stickerId) + "\". Expected \"provider:value\".");
    }

    std::string providerId = trim(trimmed.substr(0, sep));
    std::string providerValue = trim(trimmed.substr(sep + 1));

    if (providerId.empty() || providerValue.empty()) {
        throw std::invalid_argument("Invalid sticker ID format: provider or value is empty");
    }

    return ParsedStickerId{
        .providerId = std::move(providerId),
        .providerValue = std::move(providerValue)
    };
}

bool StickerIdUtils::tryParseStickerId(std::string_view stickerId, ParsedStickerId& out) noexcept {
    try {
        out = parseStickerId(stickerId);
        return true;
    } catch (...) {
        return false;
    }
}

std::string StickerIdUtils::buildStickerId(std::string_view providerId, std::string_view providerValue) {
    return std::string(providerId) + ":" + std::string(providerValue);
}

const std::vector<std::string>& StickerIdUtils::getStickerCategories() noexcept {
    static const std::vector<std::string> kCategories = {
        "all",
        "flags",
        "shapes"
    };
    return kCategories;
}

} // namespace catchim::media
