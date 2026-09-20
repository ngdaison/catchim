#pragma once

#include "core/ids/Ids.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <optional>
#include <cmath>

namespace catchim::editor {

enum class HandleCorner {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class HandleEdge {
    Left,
    Right,
    Top,
    Bottom
};

enum class TransformSessionKind {
    Idle,
    CornerScale,
    EdgeScale,
    Rotation
};

struct CornerScaleSession {
    HandleCorner corner{HandleCorner::TopLeft};
    core::TrackId trackId{core::TrackId::empty()};
    core::ClipId elementId{core::ClipId::empty()};
    double initialDistance{1.0};
    double baseWidth{100.0};
    double baseHeight{100.0};
    double initialScaleX{1.0};
    double initialScaleY{1.0};
};

struct EdgeScaleSession {
    HandleEdge edge{HandleEdge::Right};
    core::TrackId trackId{core::TrackId::empty()};
    core::ClipId elementId{core::ClipId::empty()};
    double baseWidth{100.0};
    double baseHeight{100.0};
    double rotationRad{0.0};
    double initialScaleX{1.0};
    double initialScaleY{1.0};
};

struct RotationSession {
    core::TrackId trackId{core::TrackId::empty()};
    core::ClipId elementId{core::ClipId::empty()};
    double initialAngle{0.0};
    double initialRotation{0.0};
};

class TransformHandleController {
public:
    static constexpr double MIN_SCALE = 0.001;

    TransformHandleController() = default;

    TransformSessionKind sessionKind() const noexcept { return kind_; }
    bool isIdle() const noexcept { return kind_ == TransformSessionKind::Idle; }

    static double clampScaleNonZero(double scale, double minScale = MIN_SCALE) noexcept;
    static double getCornerDistance(double width, double height, double rotationDeg, HandleCorner corner) noexcept;

    static bool shouldClearScaleAnimation(const Clip& clip) noexcept;
    static void clearScaleAnimationChannels(Clip& clip);

    void startCornerScale(
        core::TrackId trackId,
        core::ClipId elementId,
        HandleCorner corner,
        double width,
        double height,
        double rotationDeg,
        double initialScaleX = 1.0,
        double initialScaleY = 1.0
    );

    void startEdgeScale(
        core::TrackId trackId,
        core::ClipId elementId,
        HandleEdge edge,
        double width,
        double height,
        double rotationDeg,
        double initialScaleX = 1.0,
        double initialScaleY = 1.0
    );

    void startRotation(
        core::TrackId trackId,
        core::ClipId elementId,
        double pointerAngleRad,
        double initialRotationDeg
    );

    void endSession() noexcept;

    const std::optional<CornerScaleSession>& cornerSession() const noexcept { return cornerSession_; }
    const std::optional<EdgeScaleSession>& edgeSession() const noexcept { return edgeSession_; }
    const std::optional<RotationSession>& rotationSession() const noexcept { return rotationSession_; }

private:
    TransformSessionKind kind_{TransformSessionKind::Idle};
    std::optional<CornerScaleSession> cornerSession_{std::nullopt};
    std::optional<EdgeScaleSession> edgeSession_{std::nullopt};
    std::optional<RotationSession> rotationSession_{std::nullopt};
};

} // namespace catchim::editor
