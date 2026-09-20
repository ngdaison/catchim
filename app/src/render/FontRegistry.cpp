#include "render/FontRegistry.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

namespace {

std::string toLower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return out;
}

} // namespace

FontRegistry& FontRegistry::instance() {
    static FontRegistry s_instance;
    return s_instance;
}

FontRegistry::FontRegistry() {
    initSystemFonts();
}

void FontRegistry::initSystemFonts() {
    fonts_ = {
        {
            "inter",
            "Inter",
            FontCategory::SansSerif,
            {"system-ui", "-apple-system", "Segoe UI", "Roboto", "Helvetica Neue", "Arial"},
            {100, 200, 300, 400, 500, 600, 700, 800, 900},
            true
        },
        {
            "roboto",
            "Roboto",
            FontCategory::SansSerif,
            {"Inter", "Arial", "sans-serif"},
            {100, 300, 400, 500, 700, 900},
            true
        },
        {
            "montserrat",
            "Montserrat",
            FontCategory::Display,
            {"Arial", "sans-serif"},
            {300, 400, 600, 700, 800, 900},
            false
        },
        {
            "arial",
            "Arial",
            FontCategory::SansSerif,
            {"Helvetica Neue", "Helvetica", "sans-serif"},
            {400, 700},
            true
        },
        {
            "segoe-ui",
            "Segoe UI",
            FontCategory::SansSerif,
            {"Tahoma", "Geneva", "Verdana", "sans-serif"},
            {300, 400, 600, 700},
            true
        },
        {
            "times-new-roman",
            "Times New Roman",
            FontCategory::Serif,
            {"Times", "serif"},
            {400, 700},
            true
        },
        {
            "courier-new",
            "Courier New",
            FontCategory::Monospace,
            {"Courier", "monospace"},
            {400, 700},
            true
        }
    };
}

const FontFamilyItem* FontRegistry::findFont(std::string_view idOrName) const noexcept {
    std::string needle = toLower(idOrName);
    for (const auto& item : fonts_) {
        if (toLower(item.id) == needle || toLower(item.displayName) == needle) {
            return &item;
        }
    }
    return nullptr;
}

std::string FontRegistry::resolveFontFamily(std::string_view requestedFamily) const {
    const auto* matched = findFont(requestedFamily);
    if (matched) {
        return matched->displayName;
    }
    return defaultFontFamily_;
}

int FontRegistry::resolveNearestWeight(std::string_view fontId, int requestedWeight) const {
    const auto* font = findFont(fontId);
    if (!font || font->supportedWeights.empty()) {
        return 400;
    }

    int bestWeight = font->supportedWeights.front();
    int minDiff = std::abs(bestWeight - requestedWeight);

    for (int w : font->supportedWeights) {
        int diff = std::abs(w - requestedWeight);
        if (diff < minDiff) {
            minDiff = diff;
            bestWeight = w;
        }
    }

    return bestWeight;
}

} // namespace catchim::render
