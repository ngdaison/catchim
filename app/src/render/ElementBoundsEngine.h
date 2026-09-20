#pragma once

#include "editor/canvas/CanvasViewportController.h"
#include "editor/timeline/Track.h"
#include "editor/timeline/Clip.h"
#include "core/time/TimelineTime.h"
#include "render/HitTesting.h"
#include "render/PreviewSnap.h"
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <cmath>

namespace catchim::render {

using editor::ElementBounds;
using editor::BoundsCorner;
using editor::BoundsEdge;
using editor::ROTATION_HANDLE_OFFSET;

constexpr double kStickerIntrinsicFallback = 256.0;
constexpr double kDefaultGraphicSourceSize = 200.0;

struct ElementWithBounds {
    std::string trackId;
    std::string elementId;
    editor::ClipType clipType{editor::ClipType::Video};
    editor::ElementBounds bounds;
};

class ElementBoundsEngine {
public:
    static Vec2D getCornerPosition(
        const editor::ElementBounds& bounds,
        editor::BoundsCorner corner
    ) noexcept;

    static Vec2D getEdgeHandlePosition(
        const editor::ElementBounds& bounds,
        editor::BoundsEdge edge
    ) noexcept;

    static Vec2D getRotationHandlePosition(
        const editor::ElementBounds& bounds,
        double offset = editor::ROTATION_HANDLE_OFFSET
    ) noexcept;

    static std::optional<editor::ElementBounds> computeVisualBounds(
        const editor::Clip& clip,
        double canvasWidth,
        double canvasHeight,
        core::TimelineTime localTime = core::TimelineTime(0),
        std::optional<Size2D> mediaSize = std::nullopt
    );

    static std::vector<ElementWithBounds> getVisibleElementsWithBounds(
        const std::vector<editor::Track>& tracks,
        core::TimelineTime currentTime,
        double canvasWidth,
        double canvasHeight,
        const std::unordered_map<std::string, Size2D>& mediaSizes = {}
    );

    static bool pointInRotatedRect(
        double px,
        double py,
        const editor::ElementBounds& bounds
    ) noexcept;

    static std::vector<ElementWithBounds> getHitElements(
        double canvasX,
        double canvasY,
        const std::vector<ElementWithBounds>& elementsWithBounds
    );

    static std::optional<ElementWithBounds> hitTest(
        double canvasX,
        double canvasY,
        const std::vector<ElementWithBounds>& elementsWithBounds
    );

    static std::optional<ElementWithBounds> resolvePreferredHit(
        const std::vector<ElementWithBounds>& hits,
        const std::vector<std::string>& preferredElementIds
    );
};

} // namespace catchim::render
