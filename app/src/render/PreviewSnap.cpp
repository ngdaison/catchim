#include "render/PreviewSnap.h"
#include <cmath>
#include <algorithm>
#include <set>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::render {

namespace {

enum class ScaleEdge {
    Left,
    Right,
    Top,
    Bottom
};

bool hasPreferredEdge(const ScaleEdgePreference& pref, ScaleEdge edge) {
    switch (edge) {
        case ScaleEdge::Left: return pref.left;
        case ScaleEdge::Right: return pref.right;
        case ScaleEdge::Top: return pref.top;
        case ScaleEdge::Bottom: return pref.bottom;
    }
    return false;
}

} // namespace

PreviewSnapResult PreviewSnap::snapPosition(
    Point2D proposedPosition,
    Size2D canvasSize,
    Size2D elementSize,
    double rotationDegrees,
    Point2D snapThreshold
) {
    const double centerX = 0.0;
    const double centerY = 0.0;
    const double left = -canvasSize.width / 2.0;
    const double right = canvasSize.width / 2.0;
    const double top = -canvasSize.height / 2.0;
    const double bottom = canvasSize.height / 2.0;

    double rotRad = (rotationDegrees * M_PI) / 180.0;
    double cosR = std::abs(std::cos(rotRad));
    double sinR = std::abs(std::sin(rotRad));
    double halfWidth = (elementSize.width * cosR + elementSize.height * sinR) / 2.0;
    double halfHeight = (elementSize.width * sinR + elementSize.height * cosR) / 2.0;

    std::vector<SnapLine> activeLines;
    const double verticalTargets[] = {centerX, left, right};
    const double horizontalTargets[] = {centerY, top, bottom};

    double closestDistX = snapThreshold.x;
    double x = proposedPosition.x;
    bool hasLineX = false;
    SnapLine lineX{SnapLineType::Vertical, 0.0};

    const double xPoints[] = {
        proposedPosition.x,
        proposedPosition.x - halfWidth,
        proposedPosition.x + halfWidth
    };
    const double xOffsets[] = {0.0, halfWidth, -halfWidth};

    for (double targetX : verticalTargets) {
        for (int i = 0; i < 3; ++i) {
            double dist = std::abs(xPoints[i] - targetX);
            if (dist <= closestDistX) {
                closestDistX = dist;
                x = targetX + xOffsets[i];
                hasLineX = true;
                lineX = {SnapLineType::Vertical, targetX};
            }
        }
    }

    double closestDistY = snapThreshold.y;
    double y = proposedPosition.y;
    bool hasLineY = false;
    SnapLine lineY{SnapLineType::Horizontal, 0.0};

    const double yPoints[] = {
        proposedPosition.y,
        proposedPosition.y - halfHeight,
        proposedPosition.y + halfHeight
    };
    const double yOffsets[] = {0.0, halfHeight, -halfHeight};

    for (double targetY : horizontalTargets) {
        for (int i = 0; i < 3; ++i) {
            double dist = std::abs(yPoints[i] - targetY);
            if (dist <= closestDistY) {
                closestDistY = dist;
                y = targetY + yOffsets[i];
                hasLineY = true;
                lineY = {SnapLineType::Horizontal, targetY};
            }
        }
    }

    if (hasLineX) activeLines.push_back(lineX);
    if (hasLineY) activeLines.push_back(lineY);

    return {Point2D{x, y}, activeLines};
}

ScaleSnapResult PreviewSnap::snapScale(
    double proposedScale,
    Point2D position,
    double baseWidth,
    double baseHeight,
    double rotationDegrees,
    Size2D canvasSize,
    Point2D snapThreshold,
    ScaleEdgePreference preferredEdges
) {
    const double centerX = 0.0;
    const double centerY = 0.0;
    const double left = -canvasSize.width / 2.0;
    const double right = canvasSize.width / 2.0;
    const double top = -canvasSize.height / 2.0;
    const double bottom = canvasSize.height / 2.0;

    double rotRad = (rotationDegrees * M_PI) / 180.0;
    double cosR = std::abs(std::cos(rotRad));
    double sinR = std::abs(std::sin(rotRad));
    double aabbBaseHalfW = (baseWidth * cosR + baseHeight * sinR) / 2.0;
    double aabbBaseHalfH = (baseWidth * sinR + baseHeight * cosR) / 2.0;

    if (aabbBaseHalfW <= 1e-6 || aabbBaseHalfH <= 1e-6) {
        return {proposedScale, {}};
    }

    double leftEdge = position.x - aabbBaseHalfW * proposedScale;
    double rightEdge = position.x + aabbBaseHalfW * proposedScale;
    double topEdge = position.y - aabbBaseHalfH * proposedScale;
    double bottomEdge = position.y + aabbBaseHalfH * proposedScale;

    double bestScale = proposedScale;
    bool foundBest = false;
    double bestDist = 1e9;
    ScaleEdge bestEdge = ScaleEdge::Left;

    auto consider = [&](double scale, double dist, ScaleEdge edge) {
        if (std::abs(scale) <= MIN_SCALE) return;
        if (dist < bestDist) {
            bestDist = dist;
            bestScale = scale;
            bestEdge = edge;
            foundBest = true;
        } else if (dist == bestDist && foundBest) {
            bool preferNew = hasPreferredEdge(preferredEdges, edge);
            bool preferOld = hasPreferredEdge(preferredEdges, bestEdge);
            if (preferNew && !preferOld) {
                bestDist = dist;
                bestScale = scale;
                bestEdge = edge;
            }
        }
    };

    const double verticalTargets[] = {left, centerX, right};
    for (double target : verticalTargets) {
        double distLeft = std::abs(leftEdge - target);
        if (distLeft <= snapThreshold.x) {
            consider((position.x - target) / aabbBaseHalfW, distLeft, ScaleEdge::Left);
        }
        double distRight = std::abs(rightEdge - target);
        if (distRight <= snapThreshold.x) {
            consider((target - position.x) / aabbBaseHalfW, distRight, ScaleEdge::Right);
        }
    }

    const double horizontalTargets[] = {top, centerY, bottom};
    for (double target : horizontalTargets) {
        double distTop = std::abs(topEdge - target);
        if (distTop <= snapThreshold.y) {
            consider((position.y - target) / aabbBaseHalfH, distTop, ScaleEdge::Top);
        }
        double distBottom = std::abs(bottomEdge - target);
        if (distBottom <= snapThreshold.y) {
            consider((target - position.y) / aabbBaseHalfH, distBottom, ScaleEdge::Bottom);
        }
    }

    if (!foundBest) {
        return {proposedScale, {}};
    }

    double snappedLeft = position.x - aabbBaseHalfW * bestScale;
    double snappedRight = position.x + aabbBaseHalfW * bestScale;
    double snappedTop = position.y - aabbBaseHalfH * bestScale;
    double snappedBottom = position.y + aabbBaseHalfH * bestScale;

    std::vector<SnapLine> activeLines;
    for (double target : verticalTargets) {
        if (std::abs(snappedLeft - target) <= 1.0 || std::abs(snappedRight - target) <= 1.0) {
            activeLines.push_back({SnapLineType::Vertical, target});
        }
    }
    for (double target : horizontalTargets) {
        if (std::abs(snappedTop - target) <= 1.0 || std::abs(snappedBottom - target) <= 1.0) {
            activeLines.push_back({SnapLineType::Horizontal, target});
        }
    }

    return {bestScale, activeLines};
}

ScaleAxesSnapResult PreviewSnap::snapScaleAxes(
    double proposedScaleX,
    double proposedScaleY,
    Point2D position,
    double baseWidth,
    double baseHeight,
    double rotationDegrees,
    Size2D canvasSize,
    Point2D snapThreshold,
    ScaleEdgePreference preferredEdges
) {
    const double canvasLeft = -canvasSize.width / 2.0;
    const double canvasRight = canvasSize.width / 2.0;
    const double canvasTop = -canvasSize.height / 2.0;
    const double canvasBottom = canvasSize.height / 2.0;

    double rotRad = (rotationDegrees * M_PI) / 180.0;
    double cosR = std::abs(std::cos(rotRad));
    double sinR = std::abs(std::sin(rotRad));
    constexpr double EPSILON = 1e-6;

    double currentAabbHalfW = (baseWidth * proposedScaleX * cosR + baseHeight * proposedScaleY * sinR) / 2.0;
    double currentAabbHalfH = (baseWidth * proposedScaleX * sinR + baseHeight * proposedScaleY * cosR) / 2.0;
    double currentLeftEdge = position.x - currentAabbHalfW;
    double currentRightEdge = position.x + currentAabbHalfW;
    double currentTopEdge = position.y - currentAabbHalfH;
    double currentBottomEdge = position.y + currentAabbHalfH;

    double bestScaleX = proposedScaleX;
    bool foundBestX = false;
    double bestDistX = 1e9;
    ScaleEdge bestEdgeX = ScaleEdge::Left;
    SnapLine bestLineX{SnapLineType::Vertical, 0.0};

    auto considerX = [&](double scale, double dist, SnapLine line, ScaleEdge edge) {
        if (std::abs(scale) <= MIN_SCALE) return;
        if (dist < bestDistX) {
            bestDistX = dist;
            bestScaleX = scale;
            bestEdgeX = edge;
            bestLineX = line;
            foundBestX = true;
        } else if (dist == bestDistX && foundBestX) {
            bool preferNew = hasPreferredEdge(preferredEdges, edge);
            bool preferOld = hasPreferredEdge(preferredEdges, bestEdgeX);
            if (preferNew && !preferOld) {
                bestDistX = dist;
                bestScaleX = scale;
                bestEdgeX = edge;
                bestLineX = line;
            }
        }
    };

    double yContribW = baseHeight * proposedScaleY * sinR;
    double yContribH = baseHeight * proposedScaleY * cosR;

    if (cosR > EPSILON) {
        for (double T : {canvasLeft, 0.0, canvasRight}) {
            SnapLine line{SnapLineType::Vertical, T};
            double distLeft = std::abs(currentLeftEdge - T);
            if (distLeft <= snapThreshold.x) {
                considerX((2.0 * (position.x - T) - yContribW) / (baseWidth * cosR), distLeft, line, ScaleEdge::Left);
            }
            double distRight = std::abs(currentRightEdge - T);
            if (distRight <= snapThreshold.x) {
                considerX((2.0 * (T - position.x) - yContribW) / (baseWidth * cosR), distRight, line, ScaleEdge::Right);
            }
        }
    }

    if (sinR > EPSILON) {
        for (double T : {canvasTop, 0.0, canvasBottom}) {
            SnapLine line{SnapLineType::Horizontal, T};
            double distTop = std::abs(currentTopEdge - T);
            if (distTop <= snapThreshold.y) {
                considerX((2.0 * (position.y - T) - yContribH) / (baseWidth * sinR), distTop, line, ScaleEdge::Top);
            }
            double distBottom = std::abs(currentBottomEdge - T);
            if (distBottom <= snapThreshold.y) {
                considerX((2.0 * (T - position.y) - yContribH) / (baseWidth * sinR), distBottom, line, ScaleEdge::Bottom);
            }
        }
    }

    double bestScaleY = proposedScaleY;
    bool foundBestY = false;
    double bestDistY = 1e9;
    ScaleEdge bestEdgeY = ScaleEdge::Top;
    SnapLine bestLineY{SnapLineType::Horizontal, 0.0};

    auto considerY = [&](double scale, double dist, SnapLine line, ScaleEdge edge) {
        if (std::abs(scale) <= MIN_SCALE) return;
        if (dist < bestDistY) {
            bestDistY = dist;
            bestScaleY = scale;
            bestEdgeY = edge;
            bestLineY = line;
            foundBestY = true;
        } else if (dist == bestDistY && foundBestY) {
            bool preferNew = hasPreferredEdge(preferredEdges, edge);
            bool preferOld = hasPreferredEdge(preferredEdges, bestEdgeY);
            if (preferNew && !preferOld) {
                bestDistY = dist;
                bestScaleY = scale;
                bestEdgeY = edge;
                bestLineY = line;
            }
        }
    };

    double xContribW = baseWidth * proposedScaleX * cosR;
    double xContribH = baseWidth * proposedScaleX * sinR;

    if (sinR > EPSILON) {
        for (double T : {canvasLeft, 0.0, canvasRight}) {
            SnapLine line{SnapLineType::Vertical, T};
            double distLeft = std::abs(currentLeftEdge - T);
            if (distLeft <= snapThreshold.x) {
                considerY((2.0 * (position.x - T) - xContribW) / (baseHeight * sinR), distLeft, line, ScaleEdge::Left);
            }
            double distRight = std::abs(currentRightEdge - T);
            if (distRight <= snapThreshold.x) {
                considerY((2.0 * (T - position.x) - xContribW) / (baseHeight * sinR), distRight, line, ScaleEdge::Right);
            }
        }
    }

    if (cosR > EPSILON) {
        for (double T : {canvasTop, 0.0, canvasBottom}) {
            SnapLine line{SnapLineType::Horizontal, T};
            double distTop = std::abs(currentTopEdge - T);
            if (distTop <= snapThreshold.y) {
                considerY((2.0 * (position.y - T) - xContribH) / (baseHeight * cosR), distTop, line, ScaleEdge::Top);
            }
            double distBottom = std::abs(currentBottomEdge - T);
            if (distBottom <= snapThreshold.y) {
                considerY((2.0 * (T - position.y) - xContribH) / (baseHeight * cosR), distBottom, line, ScaleEdge::Bottom);
            }
        }
    }

    AxisSnapResult resX;
    resX.snappedScale = bestScaleX;
    resX.snapDistance = bestDistX;
    if (foundBestX) resX.activeLines.push_back(bestLineX);

    AxisSnapResult resY;
    resY.snappedScale = bestScaleY;
    resY.snapDistance = bestDistY;
    if (foundBestY) resY.activeLines.push_back(bestLineY);

    return {resX, resY};
}

RotationSnapResult PreviewSnap::snapRotation(double proposedRotationDegrees) {
    double nearestRotationSnap = std::round(proposedRotationDegrees / ROTATION_SNAP_STEP_DEGREES) * ROTATION_SNAP_STEP_DEGREES;
    double distanceToNearestSnap = std::abs(proposedRotationDegrees - nearestRotationSnap);
    if (distanceToNearestSnap <= ROTATION_SNAP_THRESHOLD_DEGREES) {
        return {nearestRotationSnap, true};
    }
    return {proposedRotationDegrees, false};
}

} // namespace catchim::render
