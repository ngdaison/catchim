#pragma once

#include <string>
#include <optional>
#include <utility>
#include <cstdint>

namespace catchim::core {

enum class ColorFormat {
    Hex,
    Rgb,
    Hsl,
    Hsv
};

struct RgbColor {
    uint8_t r{0};
    uint8_t g{0};
    uint8_t b{0};
    double a{1.0}; // 0.0 to 1.0
};

struct HsvColor {
    double h{0.0}; // 0 to 360
    double s{0.0}; // 0 to 1
    double v{0.0}; // 0 to 1
    double a{1.0};
};

struct HslColor {
    double h{0.0}; // 0 to 360
    double s{0.0}; // 0 to 1
    double l{0.0}; // 0 to 1
    double a{1.0};
};

class ColorUtils {
public:
    // Hex <-> RGB
    static std::optional<RgbColor> hexToRgb(const std::string& hex) noexcept;
    static std::string rgbToHex(const RgbColor& color, bool includeAlpha = false) noexcept;

    // Hex <-> HSV
    static std::optional<HsvColor> hexToHsv(const std::string& hex) noexcept;
    static std::string hsvToHex(const HsvColor& color) noexcept;

    // Hex <-> HSL
    static std::optional<HslColor> hexToHsl(const std::string& hex) noexcept;
    static std::string hslToHex(const HslColor& color) noexcept;

    // Direct RGB <-> HSV / HSL conversions
    static HsvColor rgbToHsv(const RgbColor& rgb) noexcept;
    static RgbColor hsvToRgb(const HsvColor& hsv) noexcept;
    static HslColor rgbToHsl(const RgbColor& rgb) noexcept;
    static RgbColor hslToRgb(const HslColor& hsl) noexcept;

    // Alpha extraction and appending
    static std::pair<std::string, double> parseHexAlpha(const std::string& hex) noexcept;
    static std::string appendAlpha(const std::string& rgbHex, double alpha) noexcept;

    // Text & CSS extraction
    static std::optional<std::string> extractColorFromText(const std::string& text) noexcept;

    // Formatting & Input Parsing
    static std::string formatColorValue(const std::string& hex, ColorFormat format) noexcept;
    static std::optional<std::string> parseColorInput(const std::string& input, ColorFormat format) noexcept;
};

} // namespace catchim::core
