#include "export/ExportGeometryResolver.h"
#include <algorithm>

namespace catchim::exporting {

int32_t ExportGeometryResolver::roundToEven(double val) noexcept {
    int32_t rounded = static_cast<int32_t>(std::round(val / 2.0) * 2.0);
    return std::max(2, rounded);
}

int32_t ExportGeometryResolver::getResolutionHeight(ExportResolution res) noexcept {
    switch (res) {
        case ExportResolution::Res480p:  return 480;
        case ExportResolution::Res720p:  return 720;
        case ExportResolution::Res1080p: return 1080;
        case ExportResolution::Res1440p: return 1440;
        case ExportResolution::Res2160p: return 2160;
        case ExportResolution::Res4320p: return 4320;
        case ExportResolution::Source:
        default:
            return 0;
    }
}

ExportDimensions ExportGeometryResolver::resolveDimensions(
    int32_t srcW,
    int32_t srcH,
    ExportResolution res
) noexcept {
    if (srcW <= 0 || srcH <= 0) {
        return {1920, 1080};
    }

    if (res == ExportResolution::Source) {
        return {roundToEven(srcW), roundToEven(srcH)};
    }

    int32_t targetHeight = getResolutionHeight(res);
    if (targetHeight <= 0) {
        return {roundToEven(srcW), roundToEven(srcH)};
    }

    double aspect = static_cast<double>(srcW) / srcH;
    int32_t targetWidth = roundToEven(targetHeight * aspect);

    return {targetWidth, targetHeight};
}

int64_t ExportGeometryResolver::calculateVideoBitrate(
    int32_t w,
    int32_t h,
    double fps,
    ExportQuality quality
) noexcept {
    double bpp = 0.12; // Medium default
    switch (quality) {
        case ExportQuality::Low:      bpp = 0.07; break;
        case ExportQuality::Medium:   bpp = 0.12; break;
        case ExportQuality::High:     bpp = 0.18; break;
        case ExportQuality::VeryHigh: bpp = 0.28; break;
    }

    double safeFps = std::max(1.0, fps);
    double bitsPerSec = static_cast<double>(w) * h * safeFps * bpp;
    return static_cast<int64_t>(std::round(bitsPerSec));
}

} // namespace catchim::exporting
