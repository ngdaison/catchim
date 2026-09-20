#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace catchim::render {

enum class OverlayGuideType {
    Grid,
    TikTok,
    InstagramReels,
    YouTubeShorts,
    SnapchatSpotlight
};

struct GridConfig {
    int rows{3};
    int cols{3};

    static constexpr int kMin = 1;
    static constexpr int kMax = 24;

    bool operator==(const GridConfig& other) const noexcept = default;
};

struct GridLine {
    double x1{0.0};
    double y1{0.0};
    double x2{0.0};
    double y2{0.0};

    bool operator==(const GridLine& other) const noexcept = default;
};

struct SafeZoneRect {
    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};

    bool operator==(const SafeZoneRect& other) const noexcept = default;
};

struct GuideMetadata {
    std::string id;
    std::string label;
    std::string domain;
    OverlayGuideType type;

    bool operator==(const GuideMetadata& other) const noexcept = default;
};

class GuideOverlayEngine {
public:
    // Returns list of all supported guides in the registry
    static const std::vector<GuideMetadata>& getAllGuides() noexcept;

    // Resolves guide metadata by string identifier (e.g. "grid", "tiktok", "ig-reels", "yt-shorts", "spotlight")
    static std::optional<GuideMetadata> getGuideById(std::string_view guideId) noexcept;

    // Checks whether an identifier is a registered guide
    static bool isGuideId(std::string_view guideId) noexcept;

    // Computes interior grid lines (e.g. 3x3 rule of thirds) for a canvas
    static std::vector<GridLine> calculateGridLines(
        double canvasWidth,
        double canvasHeight,
        const GridConfig& config = GridConfig{3, 3}
    ) noexcept;

    // Computes recommended content safe zone bounding box for social platforms
    static SafeZoneRect calculateSafeZone(
        double canvasWidth,
        double canvasHeight,
        OverlayGuideType guideType
    ) noexcept;

    // Helper to compute safe zone by string identifier
    static std::optional<SafeZoneRect> calculateSafeZoneById(
        double canvasWidth,
        double canvasHeight,
        std::string_view guideId
    ) noexcept;
};

} // namespace catchim::render
