#pragma once

#include <string>
#include <vector>
#include <string_view>

namespace catchim::render {

enum class FontCategory {
    SansSerif,
    Serif,
    Monospace,
    Display,
    Handwriting
};

struct FontFamilyItem {
    std::string id;
    std::string displayName;
    FontCategory category{FontCategory::SansSerif};
    std::vector<std::string> fallbackList;
    std::vector<int> supportedWeights;
    bool isSystemFont{true};
};

class FontRegistry {
public:
    static FontRegistry& instance();

    FontRegistry();

    const std::vector<FontFamilyItem>& availableFonts() const noexcept { return fonts_; }
    const FontFamilyItem* findFont(std::string_view idOrName) const noexcept;
    std::string resolveFontFamily(std::string_view requestedFamily) const;
    int resolveNearestWeight(std::string_view fontId, int requestedWeight) const;

private:
    void initSystemFonts();

    std::vector<FontFamilyItem> fonts_;
    std::string defaultFontFamily_{"Inter"};
};

} // namespace catchim::render
