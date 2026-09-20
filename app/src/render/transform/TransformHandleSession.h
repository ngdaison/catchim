#pragma once

#include "render/Transform.h"
#include "render/PreviewSnap.h"
#include "editor/canvas/CanvasViewportController.h"
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::render {

enum class HandleSessionKind {
    Idle,
    CornerScale,
    EdgeScale,
    Rotation
};

struct HandleTransformResult {
    Transform transform{};
    std::vector<SnapLine> activeSnapLines{};
    bool isSnapped{false};

    nlohmann::json toPatchParams() const {
        nlohmann::json patch;
        patch["transform.scaleX"] = transform.scaleX;
        patch["transform.scaleY"] = transform.scaleY;
        patch["transform.rotate"] = transform.rotate;
        patch["transform.positionX"] = transform.positionX;
        patch["transform.positionY"] = transform.positionY;
        return patch;
    }
};

/**
 * @brief Manages interactive gizmo dragging sessions (corner scale, edge scale, rotation)
 * with real-time snapping, matching web/src/preview/controllers/transform-handle-controller.ts.
 */
class TransformHandleSession {
public:
    TransformHandleSession() = default;

    void startCornerScale(
        editor::BoundsCorner corner,
        Vec2D startCanvasPos,
        const editor::ElementBounds& bounds,
        const Transform& initialTransform
    );

    void startEdgeScale(
        editor::BoundsEdge edge,
        Vec2D startCanvasPos,
        const editor::ElementBounds& bounds,
        const Transform& initialTransform
    );

    void startRotation(
        Vec2D startCanvasPos,
        const editor::ElementBounds& bounds,
        const Transform& initialTransform
    );

    HandleTransformResult update(Vec2D currentCanvasPos, bool uniformLock = false);

    void reset() noexcept;

    HandleSessionKind kind() const noexcept { return kind_; }
    bool isActive() const noexcept { return kind_ != HandleSessionKind::Idle; }

    const Transform& currentTransform() const noexcept { return currentTransform_; }

private:
    HandleSessionKind kind_{HandleSessionKind::Idle};
    editor::BoundsCorner corner_{editor::BoundsCorner::TopLeft};
    editor::BoundsEdge edge_{editor::BoundsEdge::Right};

    Vec2D startPos_{0.0, 0.0};
    editor::ElementBounds initialBounds_{};
    Transform initialTransform_{};
    Transform currentTransform_{};

    double initialDistance_{1.0};
    double initialAngleDegrees_{0.0};
};

} // namespace catchim::render
