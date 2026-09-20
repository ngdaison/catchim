#pragma once

#include "export/ExportSettings.h"
#include <cstdint>
#include <cmath>

namespace catchim::exporting {

enum class ExportResolution {
    Source,
    Res480p,
    Res720p,
    Res1080p,
    Res1440p,
    Res2160p,
    Res4320p
};

struct ExportDimensions {
    int32_t width{1920};
    int32_t height{1080};
};

class ExportGeometryResolver {
public:
    static int32_t roundToEven(double val) noexcept;
    static int32_t getResolutionHeight(ExportResolution res) noexcept;

    static ExportDimensions resolveDimensions(
        int32_t srcW,
        int32_t srcH,
        ExportResolution res
    ) noexcept;

    static int64_t calculateVideoBitrate(
        int32_t w,
        int32_t h,
        double fps,
        ExportQuality quality
    ) noexcept;
};

} // namespace catchim::exporting
