#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <vector>

namespace catchim::render {

struct RenderPosition {
    double x{0.0};
    double y{0.0};

    bool operator==(const RenderPosition& other) const noexcept {
        return x == other.x && y == other.y;
    }
    bool operator!=(const RenderPosition& other) const noexcept {
        return !(*this == other);
    }
};

struct RenderTransform {
    double scaleX{1.0};
    double scaleY{1.0};
    RenderPosition position{0.0, 0.0};
    double rotate{0.0};

    bool operator==(const RenderTransform& other) const noexcept {
        return scaleX == other.scaleX &&
               scaleY == other.scaleY &&
               position == other.position &&
               rotate == other.rotate;
    }
    bool operator!=(const RenderTransform& other) const noexcept {
        return !(*this == other);
    }
};

enum class BlendMode {
    Normal,
    Darken,
    Multiply,
    ColorBurn,
    Lighten,
    Screen,
    PlusLighter,
    ColorDodge,
    Overlay,
    SoftLight,
    HardLight,
    Difference,
    Exclusion,
    Hue,
    Saturation,
    Color,
    Luminosity
};

class RenderingParamsResolver {
public:
    static const std::vector<std::string>& allBlendModes() noexcept;

    static std::string blendModeToString(BlendMode mode);
    static std::optional<BlendMode> blendModeFromString(const std::string& str);
    static bool isBlendMode(const std::string& value) noexcept;

    static RenderTransform buildTransformFromParams(
        const std::unordered_map<std::string, double>& params
    ) noexcept;

    static double readOpacityFromParams(
        const std::unordered_map<std::string, double>& params,
        double fallback = 1.0
    ) noexcept;

    static std::string readBlendModeFromParams(
        const std::unordered_map<std::string, std::string>& params,
        const std::string& fallback = "normal"
    );
};

} // namespace catchim::render
