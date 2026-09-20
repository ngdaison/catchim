#include "render/guides/GuideOverlayEngine.h"
#include <algorithm>

namespace catchim::render {

const std::vector<GuideMetadata>& GuideOverlayEngine::getAllGuides() noexcept {
    static const std::vector<GuideMetadata> s_guides = {
        {"grid",      "Grid",      "",              OverlayGuideType::Grid},
        {"tiktok",    "TikTok",    "tiktok.com",    OverlayGuideType::TikTok},
        {"ig-reels",  "Reels",     "instagram.com", OverlayGuideType::InstagramReels},
        {"yt-shorts", "Shorts",    "youtube.com",   OverlayGuideType::YouTubeShorts},
        {"spotlight", "Spotlight", "snapchat.com",  OverlayGuideType::SnapchatSpotlight}
    };
    return s_guides;
}

std::optional<GuideMetadata> GuideOverlayEngine::getGuideById(std::string_view guideId) noexcept {
    for (const auto& guide : getAllGuides()) {
        if (guide.id == guideId) {
            return guide;
        }
    }
    return std::nullopt;
}

bool GuideOverlayEngine::isGuideId(std::string_view guideId) noexcept {
    return getGuideById(guideId).has_value();
}

std::vector<GridLine> GuideOverlayEngine::calculateGridLines(
    double canvasWidth,
    double canvasHeight,
    const GridConfig& config
) noexcept {
    std::vector<GridLine> lines;
    if (canvasWidth <= 0.0 || canvasHeight <= 0.0) {
        return lines;
    }

    int rows = std::clamp(config.rows, GridConfig::kMin, GridConfig::kMax);
    int cols = std::clamp(config.cols, GridConfig::kMin, GridConfig::kMax);

    lines.reserve(static_cast<size_t>((cols - 1) + (rows - 1)));

    // Vertical grid lines
    for (int i = 1; i < cols; ++i) {
        double x = (static_cast<double>(i) / static_cast<double>(cols)) * canvasWidth;
        lines.push_back(GridLine{x, 0.0, x, canvasHeight});
    }

    // Horizontal grid lines
    for (int i = 1; i < rows; ++i) {
        double y = (static_cast<double>(i) / static_cast<double>(rows)) * canvasHeight;
        lines.push_back(GridLine{0.0, y, canvasWidth, y});
    }

    return lines;
}

SafeZoneRect GuideOverlayEngine::calculateSafeZone(
    double canvasWidth,
    double canvasHeight,
    OverlayGuideType guideType
) noexcept {
    if (canvasWidth <= 0.0 || canvasHeight <= 0.0) {
        return SafeZoneRect{0.0, 0.0, 0.0, 0.0};
    }

    double topPct = 0.0;
    double bottomPct = 0.0;
    double leftPct = 0.0;
    double rightPct = 0.0;

    switch (guideType) {
        case OverlayGuideType::TikTok:
            topPct = 0.10;
            bottomPct = 0.22;
            leftPct = 0.05;
            rightPct = 0.15;
            break;

        case OverlayGuideType::InstagramReels:
            topPct = 0.08;
            bottomPct = 0.18;
            leftPct = 0.05;
            rightPct = 0.14;
            break;

        case OverlayGuideType::YouTubeShorts:
            topPct = 0.08;
            bottomPct = 0.18;
            leftPct = 0.05;
            rightPct = 0.16;
            break;

        case OverlayGuideType::SnapchatSpotlight:
            topPct = 0.10;
            bottomPct = 0.16;
            leftPct = 0.05;
            rightPct = 0.14;
            break;

        case OverlayGuideType::Grid:
        default:
            return SafeZoneRect{0.0, 0.0, canvasWidth, canvasHeight};
    }

    double x = canvasWidth * leftPct;
    double y = canvasHeight * topPct;
    double width = canvasWidth * (1.0 - leftPct - rightPct);
    double height = canvasHeight * (1.0 - topPct - bottomPct);

    return SafeZoneRect{
        x,
        y,
        std::max(0.0, width),
        std::max(0.0, height)
    };
}

std::optional<SafeZoneRect> GuideOverlayEngine::calculateSafeZoneById(
    double canvasWidth,
    double canvasHeight,
    std::string_view guideId
) noexcept {
    auto meta = getGuideById(guideId);
    if (!meta.has_value()) {
        return std::nullopt;
    }
    return calculateSafeZone(canvasWidth, canvasHeight, meta->type);
}

} // namespace catchim::render
