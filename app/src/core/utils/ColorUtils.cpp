#include "core/utils/ColorUtils.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>

namespace catchim::core {

namespace {

std::string cleanHex(const std::string& hex) {
    std::string s = hex;
    if (!s.empty() && s[0] == '#') {
        s = s.substr(1);
    }
    // Expand 3 or 4 hex chars
    if (s.size() == 3) {
        std::string exp;
        exp += s[0]; exp += s[0];
        exp += s[1]; exp += s[1];
        exp += s[2]; exp += s[2];
        return exp;
    }
    if (s.size() == 4) {
        std::string exp;
        exp += s[0]; exp += s[0];
        exp += s[1]; exp += s[1];
        exp += s[2]; exp += s[2];
        exp += s[3]; exp += s[3];
        return exp;
    }
    return s;
}

uint8_t parseByte(const std::string& str, size_t pos) {
    if (pos + 2 > str.size()) return 0;
    std::string sub = str.substr(pos, 2);
    return static_cast<uint8_t>(std::strtoul(sub.c_str(), nullptr, 16));
}

std::string byteToHex(uint8_t b) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(b);
    return ss.str();
}

} // anonymous namespace

std::optional<RgbColor> ColorUtils::hexToRgb(const std::string& hex) noexcept {
    std::string cleaned = cleanHex(hex);
    if (cleaned.size() != 6 && cleaned.size() != 8) {
        return std::nullopt;
    }

    for (char c : cleaned) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) return std::nullopt;
    }

    RgbColor c;
    c.r = parseByte(cleaned, 0);
    c.g = parseByte(cleaned, 2);
    c.b = parseByte(cleaned, 4);
    if (cleaned.size() == 8) {
        uint8_t aByte = parseByte(cleaned, 6);
        c.a = static_cast<double>(aByte) / 255.0;
    } else {
        c.a = 1.0;
    }
    return c;
}

std::string ColorUtils::rgbToHex(const RgbColor& color, bool includeAlpha) noexcept {
    std::string res = byteToHex(color.r) + byteToHex(color.g) + byteToHex(color.b);
    if (includeAlpha) {
        uint8_t aByte = static_cast<uint8_t>(std::clamp(std::round(color.a * 255.0), 0.0, 255.0));
        res += byteToHex(aByte);
    }
    return res;
}

HsvColor ColorUtils::rgbToHsv(const RgbColor& rgb) noexcept {
    double r = rgb.r / 255.0;
    double g = rgb.g / 255.0;
    double b = rgb.b / 255.0;

    double cmax = std::max({r, g, b});
    double cmin = std::min({r, g, b});
    double delta = cmax - cmin;

    HsvColor hsv;
    hsv.a = rgb.a;
    hsv.v = cmax;

    if (delta <= 1e-6) {
        hsv.h = 0.0;
        hsv.s = 0.0;
        return hsv;
    }

    hsv.s = (cmax > 0.0) ? (delta / cmax) : 0.0;

    if (std::abs(cmax - r) < 1e-6) {
        hsv.h = 60.0 * std::fmod((g - b) / delta, 6.0);
    } else if (std::abs(cmax - g) < 1e-6) {
        hsv.h = 60.0 * (((b - r) / delta) + 2.0);
    } else {
        hsv.h = 60.0 * (((r - g) / delta) + 4.0);
    }

    if (hsv.h < 0.0) hsv.h += 360.0;
    return hsv;
}

RgbColor ColorUtils::hsvToRgb(const HsvColor& hsv) noexcept {
    double h = std::fmod(hsv.h, 360.0);
    if (h < 0.0) h += 360.0;
    double s = std::clamp(hsv.s, 0.0, 1.0);
    double v = std::clamp(hsv.v, 0.0, 1.0);

    double c = v * s;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double m = v - c;

    double r1 = 0, g1 = 0, b1 = 0;
    if (h < 60.0)       { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120.0) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180.0) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240.0) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300.0) { r1 = x; g1 = 0; b1 = c; }
    else                { r1 = c; g1 = 0; b1 = x; }

    RgbColor rgb;
    rgb.r = static_cast<uint8_t>(std::clamp(std::round((r1 + m) * 255.0), 0.0, 255.0));
    rgb.g = static_cast<uint8_t>(std::clamp(std::round((g1 + m) * 255.0), 0.0, 255.0));
    rgb.b = static_cast<uint8_t>(std::clamp(std::round((b1 + m) * 255.0), 0.0, 255.0));
    rgb.a = hsv.a;
    return rgb;
}

HslColor ColorUtils::rgbToHsl(const RgbColor& rgb) noexcept {
    double r = rgb.r / 255.0;
    double g = rgb.g / 255.0;
    double b = rgb.b / 255.0;

    double cmax = std::max({r, g, b});
    double cmin = std::min({r, g, b});
    double delta = cmax - cmin;

    HslColor hsl;
    hsl.a = rgb.a;
    hsl.l = (cmax + cmin) * 0.5;

    if (delta <= 1e-6) {
        hsl.h = 0.0;
        hsl.s = 0.0;
        return hsl;
    }

    hsl.s = delta / (1.0 - std::abs(2.0 * hsl.l - 1.0));

    if (std::abs(cmax - r) < 1e-6) {
        hsl.h = 60.0 * std::fmod((g - b) / delta, 6.0);
    } else if (std::abs(cmax - g) < 1e-6) {
        hsl.h = 60.0 * (((b - r) / delta) + 2.0);
    } else {
        hsl.h = 60.0 * (((r - g) / delta) + 4.0);
    }

    if (hsl.h < 0.0) hsl.h += 360.0;
    return hsl;
}

RgbColor ColorUtils::hslToRgb(const HslColor& hsl) noexcept {
    double h = std::fmod(hsl.h, 360.0);
    if (h < 0.0) h += 360.0;
    double s = std::clamp(hsl.s, 0.0, 1.0);
    double l = std::clamp(hsl.l, 0.0, 1.0);

    double c = (1.0 - std::abs(2.0 * l - 1.0)) * s;
    double x = c * (1.0 - std::abs(std::fmod(h / 60.0, 2.0) - 1.0));
    double m = l - c * 0.5;

    double r1 = 0, g1 = 0, b1 = 0;
    if (h < 60.0)       { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120.0) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180.0) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240.0) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300.0) { r1 = x; g1 = 0; b1 = c; }
    else                { r1 = c; g1 = 0; b1 = x; }

    RgbColor rgb;
    rgb.r = static_cast<uint8_t>(std::clamp(std::round((r1 + m) * 255.0), 0.0, 255.0));
    rgb.g = static_cast<uint8_t>(std::clamp(std::round((g1 + m) * 255.0), 0.0, 255.0));
    rgb.b = static_cast<uint8_t>(std::clamp(std::round((b1 + m) * 255.0), 0.0, 255.0));
    rgb.a = hsl.a;
    return rgb;
}

std::optional<HsvColor> ColorUtils::hexToHsv(const std::string& hex) noexcept {
    auto rgb = hexToRgb(hex);
    if (!rgb.has_value()) return std::nullopt;
    return rgbToHsv(rgb.value());
}

std::string ColorUtils::hsvToHex(const HsvColor& color) noexcept {
    auto rgb = hsvToRgb(color);
    return rgbToHex(rgb, color.a < 1.0);
}

std::optional<HslColor> ColorUtils::hexToHsl(const std::string& hex) noexcept {
    auto rgb = hexToRgb(hex);
    if (!rgb.has_value()) return std::nullopt;
    return rgbToHsl(rgb.value());
}

std::string ColorUtils::hslToHex(const HslColor& color) noexcept {
    auto rgb = hslToRgb(color);
    return rgbToHex(rgb, color.a < 1.0);
}

std::pair<std::string, double> ColorUtils::parseHexAlpha(const std::string& hex) noexcept {
    std::string cleaned = cleanHex(hex);
    if (cleaned.size() == 8) {
        uint8_t aByte = parseByte(cleaned, 6);
        return {cleaned.substr(0, 6), static_cast<double>(aByte) / 255.0};
    }
    if (cleaned.size() == 6) {
        return {cleaned, 1.0};
    }
    return {cleaned, 1.0};
}

std::string ColorUtils::appendAlpha(const std::string& rgbHex, double alpha) noexcept {
    std::string cleaned = cleanHex(rgbHex);
    if (cleaned.size() > 6) cleaned = cleaned.substr(0, 6);
    if (alpha >= 1.0) return cleaned;
    uint8_t aByte = static_cast<uint8_t>(std::clamp(std::round(alpha * 255.0), 0.0, 255.0));
    return cleaned + byteToHex(aByte);
}

std::optional<std::string> ColorUtils::extractColorFromText(const std::string& text) noexcept {
    std::string s = text;
    // Strip CSS noise like !important, trailing semicolons, leading property names
    static const std::regex noiseRegex(R"(\s*!important\s*|;+\s*$)", std::regex::icase);
    s = std::regex_replace(s, noiseRegex, "");

    // Trim
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());

    size_t colon = s.find(':');
    if (colon != std::string::npos && (s.find('(') == std::string::npos || colon < s.find('('))) {
        s = s.substr(colon + 1);
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    }

    // Check direct hex
    if (!s.empty() && s[0] == '#') {
        std::string candidate = s.substr(1);
        auto rgb = hexToRgb(candidate);
        if (rgb.has_value()) return cleanHex(candidate);
    }

    // Bare hex match (3 to 8 hex characters)
    static const std::regex bareHexRegex(R"(^[0-9a-fA-F]{3,8}$)");
    if (std::regex_match(s, bareHexRegex)) {
        auto rgb = hexToRgb(s);
        if (rgb.has_value()) return cleanHex(s);
    }

    // Fallback embedded search for #hex
    static const std::regex embeddedHexRegex(R"(#([0-9a-fA-F]{3,8})\b)");
    std::smatch m;
    if (std::regex_search(text, m, embeddedHexRegex)) {
        auto rgb = hexToRgb(m[1].str());
        if (rgb.has_value()) return cleanHex(m[1].str());
    }

    return std::nullopt;
}

std::string ColorUtils::formatColorValue(const std::string& hex, ColorFormat format) noexcept {
    auto rgbOpt = hexToRgb(hex);
    if (!rgbOpt.has_value()) return cleanHex(hex);
    const auto& rgb = rgbOpt.value();

    switch (format) {
        case ColorFormat::Hex:
            return cleanHex(hex);
        case ColorFormat::Rgb: {
            std::stringstream ss;
            ss << static_cast<int>(rgb.r) << ", "
               << static_cast<int>(rgb.g) << ", "
               << static_cast<int>(rgb.b);
            return ss.str();
        }
        case ColorFormat::Hsl: {
            HslColor hsl = rgbToHsl(rgb);
            std::stringstream ss;
            ss << static_cast<int>(std::round(hsl.h)) << ", "
               << static_cast<int>(std::round(hsl.s * 100.0)) << "%, "
               << static_cast<int>(std::round(hsl.l * 100.0)) << "%";
            return ss.str();
        }
        case ColorFormat::Hsv: {
            HsvColor hsv = rgbToHsv(rgb);
            std::stringstream ss;
            ss << static_cast<int>(std::round(hsv.h)) << ", "
               << static_cast<int>(std::round(hsv.s * 100.0)) << "%, "
               << static_cast<int>(std::round(hsv.v * 100.0)) << "%";
            return ss.str();
        }
    }
    return cleanHex(hex);
}

std::optional<std::string> ColorUtils::parseColorInput(const std::string& input, ColorFormat format) noexcept {
    std::string s = input;
    if (!s.empty() && s[0] == '#') s = s.substr(1);

    switch (format) {
        case ColorFormat::Hex: {
            auto rgb = hexToRgb(s);
            if (rgb.has_value()) return cleanHex(s);
            return std::nullopt;
        }
        case ColorFormat::Rgb: {
            // "255, 128, 0"
            std::stringstream ss(s);
            int r, g, b;
            char c1, c2;
            if ((ss >> r >> c1 >> g >> c2 >> b) && c1 == ',' && c2 == ',') {
                RgbColor rgb;
                rgb.r = static_cast<uint8_t>(std::clamp(r, 0, 255));
                rgb.g = static_cast<uint8_t>(std::clamp(g, 0, 255));
                rgb.b = static_cast<uint8_t>(std::clamp(b, 0, 255));
                return rgbToHex(rgb, false);
            }
            return std::nullopt;
        }
        case ColorFormat::Hsl: {
            // "180, 50%, 50%" or "180, 50, 50"
            std::string cleaned;
            for (char ch : s) {
                if (ch != '%') cleaned += ch;
            }
            std::stringstream ss(cleaned);
            double h, sVal, lVal;
            char c1, c2;
            if ((ss >> h >> c1 >> sVal >> c2 >> lVal) && c1 == ',' && c2 == ',') {
                HslColor hsl;
                hsl.h = h;
                hsl.s = sVal / 100.0;
                hsl.l = lVal / 100.0;
                return hslToHex(hsl);
            }
            return std::nullopt;
        }
        case ColorFormat::Hsv: {
            std::string cleaned;
            for (char ch : s) {
                if (ch != '%') cleaned += ch;
            }
            std::stringstream ss(cleaned);
            double h, sVal, vVal;
            char c1, c2;
            if ((ss >> h >> c1 >> sVal >> c2 >> vVal) && c1 == ',' && c2 == ',') {
                HsvColor hsv;
                hsv.h = h;
                hsv.s = sVal / 100.0;
                hsv.v = vVal / 100.0;
                return hsvToHex(hsv);
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}

} // namespace catchim::core
