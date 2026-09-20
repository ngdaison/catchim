#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace catchim::render {

enum class CssGradientType {
    Linear,
    RepeatingLinear,
    Radial,
    RepeatingRadial
};

enum class RadialShape {
    Circle,
    Ellipse
};

enum class RadialExtent {
    ClosestSide,
    FarthestSide,
    ClosestCorner,
    FarthestCorner
};

struct CssRgba {
    uint8_t r{255};
    uint8_t g{255};
    uint8_t b{255};
    double a{1.0};

    std::string toRgbaString() const;
    std::string toHexString() const;
    bool isTransparent() const noexcept { return a <= 0.0001; }
};

struct CssColorStop {
    CssRgba color;
    std::string rawColor;
    std::optional<double> offset; // 0.0 to 1.0
};

struct LinearPoints {
    double x0{0.0};
    double y0{0.0};
    double x1{0.0};
    double y1{0.0};
    double length{0.0};
};

struct RadialDimensions {
    double cx{0.0};
    double cy{0.0};
    double rx{0.0};
    double ry{0.0};
};

struct ParsedGradient {
    CssGradientType type{CssGradientType::Linear};
    double angleDegrees{180.0}; // For linear gradients (180deg = to bottom)
    RadialShape radialShape{RadialShape::Ellipse};
    RadialExtent radialExtent{RadialExtent::FarthestCorner};
    double centerXRatio{0.5}; // 0.0 to 1.0 (0.5 = center)
    double centerYRatio{0.5};
    std::vector<CssColorStop> colorStops;
};

enum class BackgroundLayerType {
    Color,
    Gradient
};

struct BackgroundLayer {
    BackgroundLayerType type{BackgroundLayerType::Color};
    std::string colorValue;
    ParsedGradient gradient;
};

class CssGradientEngine {
public:
    static std::vector<std::string> splitCssLayers(const std::string& css);
    static std::vector<BackgroundLayer> parseBackgroundLayers(const std::string& css);
    static std::optional<ParsedGradient> parseGradient(const std::string& css);

    static LinearPoints resolveLinearPoints(
        double width,
        double height,
        double angleDegrees
    );

    static RadialDimensions resolveRadialDimensions(
        double width,
        double height,
        RadialShape shape,
        RadialExtent extent,
        double centerXRatio,
        double centerYRatio
    );

    static std::vector<CssColorStop> normalizeColorStops(
        const std::vector<CssColorStop>& stops
    );

    static std::vector<CssColorStop> fixTransparentStops(
        const std::vector<CssColorStop>& stops
    );

    static CssRgba parseColor(const std::string& str);

    // Curated presets from web/src/data/colors
    static const std::vector<std::string>& getPatternCraftGradients();
    static const std::vector<std::string>& getSolidColorPalette();
};

} // namespace catchim::render
