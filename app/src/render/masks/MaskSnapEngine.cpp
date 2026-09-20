#include "render/masks/MaskSnapEngine.h"
#include <algorithm>
#include <cmath>

namespace catchim::render {

std::vector<SnapLine> MaskSnapEngine::toGlobalMaskSnapLines(
    const std::vector<SnapLine>& lines,
    const ElementBounds& bounds,
    const Size2D& canvasSize
) {
    double centerX = bounds.cx - canvasSize.width / 2.0;
    double centerY = bounds.cy - canvasSize.height / 2.0;
    std::vector<SnapLine> result;
    result.reserve(lines.size());
    for (const auto& line : lines) {
        result.push_back(SnapLine{
            .type = line.type,
            .position = (line.type == SnapLineType::Vertical)
                ? (centerX + line.position)
                : (centerY + line.position)
        });
    }
    return result;
}

Point2D MaskSnapEngine::getMaskLocalCenter(
    double centerX,
    double centerY,
    const ElementBounds& bounds
) noexcept {
    return Point2D{
        .x = centerX * bounds.width,
        .y = centerY * bounds.height
    };
}

std::pair<double, double> MaskSnapEngine::setMaskLocalCenter(
    const Point2D& center,
    const ElementBounds& bounds
) noexcept {
    return {
        (bounds.width == 0.0) ? 0.0 : (center.x / bounds.width),
        (bounds.height == 0.0) ? 0.0 : (center.y / bounds.height)
    };
}

MaskSnapGeometry MaskSnapEngine::getMaskSnapGeometry(
    const RectangleMaskParams& params,
    const ElementBounds& bounds
) noexcept {
    Point2D pos = getMaskLocalCenter(params.centerX, params.centerY, bounds);
    Size2D size{
        .width = std::max(params.width, BuiltinMaskGeometry::MIN_MASK_DIMENSION) * bounds.width,
        .height = std::max(params.height, BuiltinMaskGeometry::MIN_MASK_DIMENSION) * bounds.height
    };
    return MaskSnapGeometry{
        .position = pos,
        .size = size,
        .rotation = params.rotation
    };
}

MaskSnapGeometry MaskSnapEngine::getSplitMaskSnapGeometry(
    const SplitMaskParams& params,
    const ElementBounds& bounds
) noexcept {
    Point2D pos = getMaskLocalCenter(params.centerX, params.centerY, bounds);
    return MaskSnapGeometry{
        .position = pos,
        .size = Size2D{0.0, 0.0},
        .rotation = params.rotation
    };
}

MaskSnapResult MaskSnapEngine::snapBoxMaskInteraction(
    const std::string& handleKind,
    const std::string& sideOrCorner,
    const RectangleMaskParams& startParams,
    const RectangleMaskParams& proposedParams,
    const ElementBounds& bounds,
    const Size2D& canvasSize,
    Point2D snapThreshold
) {
    auto geom = getMaskSnapGeometry(proposedParams, bounds);
    Size2D localCanvasSize{ .width = bounds.width, .height = bounds.height };

    if (handleKind == "position") {
        auto snapRes = PreviewSnap::snapPosition(
            geom.position,
            localCanvasSize,
            geom.size,
            geom.rotation,
            snapThreshold
        );

        auto newCenter = setMaskLocalCenter(snapRes.snappedPosition, bounds);
        RectangleMaskParams outParams = proposedParams;
        outParams.centerX = newCenter.first;
        outParams.centerY = newCenter.second;

        return MaskSnapResult{
            .params = outParams,
            .activeLines = toGlobalMaskSnapLines(snapRes.activeLines, bounds, canvasSize)
        };
    }

    if (handleKind == "rotation") {
        auto rotRes = PreviewSnap::snapRotation(proposedParams.rotation);
        RectangleMaskParams outParams = proposedParams;
        outParams.rotation = rotRes.snappedRotation;
        return MaskSnapResult{
            .params = outParams,
            .activeLines = {}
        };
    }

    double baseWidth = std::max(startParams.width, BuiltinMaskGeometry::MIN_MASK_DIMENSION) * bounds.width;
    double baseHeight = std::max(startParams.height, BuiltinMaskGeometry::MIN_MASK_DIMENSION) * bounds.height;

    if (handleKind == "edge") {
        if (sideOrCorner == "left" || sideOrCorner == "right") {
            double proposedScaleX = std::max(proposedParams.width, BuiltinMaskGeometry::MIN_MASK_DIMENSION) /
                                    std::max(startParams.width, BuiltinMaskGeometry::MIN_MASK_DIMENSION);
            ScaleEdgePreference pref{
                .left = (sideOrCorner == "left"),
                .right = (sideOrCorner == "right")
            };
            auto axesRes = PreviewSnap::snapScaleAxes(
                proposedScaleX,
                1.0,
                geom.position,
                baseWidth,
                baseHeight,
                proposedParams.rotation,
                localCanvasSize,
                snapThreshold,
                pref
            );
            RectangleMaskParams outParams = proposedParams;
            outParams.width = std::max(
                BuiltinMaskGeometry::MIN_MASK_DIMENSION,
                startParams.width * axesRes.x.snappedScale
            );
            return MaskSnapResult{
                .params = outParams,
                .activeLines = toGlobalMaskSnapLines(axesRes.x.activeLines, bounds, canvasSize)
            };
        }

        if (sideOrCorner == "top" || sideOrCorner == "bottom") {
            double proposedScaleY = std::max(proposedParams.height, BuiltinMaskGeometry::MIN_MASK_DIMENSION) /
                                    std::max(startParams.height, BuiltinMaskGeometry::MIN_MASK_DIMENSION);
            ScaleEdgePreference pref{
                .top = (sideOrCorner == "top"),
                .bottom = (sideOrCorner == "bottom")
            };
            auto axesRes = PreviewSnap::snapScaleAxes(
                1.0,
                proposedScaleY,
                geom.position,
                baseWidth,
                baseHeight,
                proposedParams.rotation,
                localCanvasSize,
                snapThreshold,
                pref
            );
            RectangleMaskParams outParams = proposedParams;
            outParams.height = std::max(
                BuiltinMaskGeometry::MIN_MASK_DIMENSION,
                startParams.height * axesRes.y.snappedScale
            );
            return MaskSnapResult{
                .params = outParams,
                .activeLines = toGlobalMaskSnapLines(axesRes.y.activeLines, bounds, canvasSize)
            };
        }
    }

    if (handleKind == "scale") {
        double baseScale = std::max(startParams.scale, BuiltinMaskGeometry::MIN_MASK_DIMENSION);
        double proposedScale = std::max(proposedParams.scale, BuiltinMaskGeometry::MIN_MASK_DIMENSION) / baseScale;
        auto scaleRes = PreviewSnap::snapScale(
            proposedScale,
            geom.position,
            baseWidth * baseScale,
            baseHeight * baseScale,
            proposedParams.rotation,
            localCanvasSize,
            snapThreshold
        );
        RectangleMaskParams outParams = proposedParams;
        outParams.scale = std::max(
            BuiltinMaskGeometry::MIN_MASK_DIMENSION,
            startParams.scale * scaleRes.snappedScale
        );
        return MaskSnapResult{
            .params = outParams,
            .activeLines = toGlobalMaskSnapLines(scaleRes.activeLines, bounds, canvasSize)
        };
    }

    if (handleKind == "corner") {
        double proposedScale = std::max(proposedParams.width, BuiltinMaskGeometry::MIN_MASK_DIMENSION) /
                               std::max(startParams.width, BuiltinMaskGeometry::MIN_MASK_DIMENSION);
        ScaleEdgePreference pref{
            .left = (sideOrCorner.find("left") != std::string::npos),
            .right = (sideOrCorner.find("right") != std::string::npos),
            .top = (sideOrCorner.find("top") != std::string::npos),
            .bottom = (sideOrCorner.find("bottom") != std::string::npos)
        };
        auto scaleRes = PreviewSnap::snapScale(
            proposedScale,
            geom.position,
            baseWidth,
            baseHeight,
            proposedParams.rotation,
            localCanvasSize,
            snapThreshold,
            pref
        );
        RectangleMaskParams outParams = proposedParams;
        outParams.width = std::max(
            BuiltinMaskGeometry::MIN_MASK_DIMENSION,
            startParams.width * scaleRes.snappedScale
        );
        outParams.height = std::max(
            BuiltinMaskGeometry::MIN_MASK_DIMENSION,
            startParams.height * scaleRes.snappedScale
        );
        return MaskSnapResult{
            .params = outParams,
            .activeLines = toGlobalMaskSnapLines(scaleRes.activeLines, bounds, canvasSize)
        };
    }

    return MaskSnapResult{ .params = proposedParams, .activeLines = {} };
}

SplitMaskSnapResult MaskSnapEngine::snapSplitMaskInteraction(
    const std::string& handleKind,
    const SplitMaskParams& proposedParams,
    const ElementBounds& bounds,
    const Size2D& canvasSize,
    Point2D snapThreshold
) {
    auto geom = getSplitMaskSnapGeometry(proposedParams, bounds);
    Size2D localCanvasSize{ .width = bounds.width, .height = bounds.height };

    if (handleKind == "position") {
        auto snapRes = PreviewSnap::snapPosition(
            geom.position,
            localCanvasSize,
            geom.size,
            geom.rotation,
            snapThreshold
        );
        auto newCenter = setMaskLocalCenter(snapRes.snappedPosition, bounds);
        SplitMaskParams outParams = proposedParams;
        outParams.centerX = newCenter.first;
        outParams.centerY = newCenter.second;
        return SplitMaskSnapResult{
            .params = outParams,
            .activeLines = toGlobalMaskSnapLines(snapRes.activeLines, bounds, canvasSize)
        };
    }

    if (handleKind == "rotation") {
        auto rotRes = PreviewSnap::snapRotation(proposedParams.rotation);
        SplitMaskParams outParams = proposedParams;
        outParams.rotation = rotRes.snappedRotation;
        return SplitMaskSnapResult{
            .params = outParams,
            .activeLines = {}
        };
    }

    return SplitMaskSnapResult{ .params = proposedParams, .activeLines = {} };
}

} // namespace catchim::render
