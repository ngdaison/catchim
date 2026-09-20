#pragma once

#include "render/HitTesting.h"
#include <vector>
#include <string>

namespace catchim::render {

enum class SnapLineType {
    Horizontal,
    Vertical
};

struct SnapLine {
    SnapLineType type;
    double position{0.0};

    bool operator==(const SnapLine& other) const {
        return type == other.type && std::abs(position - other.position) < 0.001;
    }
};

struct Size2D {
    double width{0.0};
    double height{0.0};
};

struct PreviewSnapResult {
    Point2D snappedPosition;
    std::vector<SnapLine> activeLines;
};

struct ScaleSnapResult {
    double snappedScale{1.0};
    std::vector<SnapLine> activeLines;
};

struct AxisSnapResult {
    double snappedScale{1.0};
    double snapDistance{1e9};
    std::vector<SnapLine> activeLines;
};

struct ScaleAxesSnapResult {
    AxisSnapResult x;
    AxisSnapResult y;
};

struct RotationSnapResult {
    double snappedRotation{0.0};
    bool isSnapped{false};
};

struct ScaleEdgePreference {
    bool left{false};
    bool right{false};
    bool top{false};
    bool bottom{false};
};

class PreviewSnap {
public:
    static constexpr double ROTATION_SNAP_STEP_DEGREES = 90.0;
    static constexpr double ROTATION_SNAP_THRESHOLD_DEGREES = 5.0;
    static constexpr double MIN_SCALE = 0.01;
    static constexpr double DEFAULT_SNAP_THRESHOLD = 8.0;

    static PreviewSnapResult snapPosition(
        Point2D proposedPosition,
        Size2D canvasSize,
        Size2D elementSize,
        double rotationDegrees = 0.0,
        Point2D snapThreshold = {DEFAULT_SNAP_THRESHOLD, DEFAULT_SNAP_THRESHOLD}
    );

    static ScaleSnapResult snapScale(
        double proposedScale,
        Point2D position,
        double baseWidth,
        double baseHeight,
        double rotationDegrees,
        Size2D canvasSize,
        Point2D snapThreshold = {DEFAULT_SNAP_THRESHOLD, DEFAULT_SNAP_THRESHOLD},
        ScaleEdgePreference preferredEdges = {}
    );

    static ScaleAxesSnapResult snapScaleAxes(
        double proposedScaleX,
        double proposedScaleY,
        Point2D position,
        double baseWidth,
        double baseHeight,
        double rotationDegrees,
        Size2D canvasSize,
        Point2D snapThreshold = {DEFAULT_SNAP_THRESHOLD, DEFAULT_SNAP_THRESHOLD},
        ScaleEdgePreference preferredEdges = {}
    );

    static RotationSnapResult snapRotation(double proposedRotationDegrees);
};

} // namespace catchim::render
