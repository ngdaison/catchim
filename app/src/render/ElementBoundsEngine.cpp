#include "render/ElementBoundsEngine.h"
#include <numbers>
#include <algorithm>

namespace catchim::render {

Vec2D ElementBoundsEngine::getCornerPosition(
    const editor::ElementBounds& bounds,
    editor::BoundsCorner corner
) noexcept {
    return bounds.getCornerPosition(corner);
}

Vec2D ElementBoundsEngine::getEdgeHandlePosition(
    const editor::ElementBounds& bounds,
    editor::BoundsEdge edge
) noexcept {
    return bounds.getEdgeHandlePosition(edge);
}

Vec2D ElementBoundsEngine::getRotationHandlePosition(
    const editor::ElementBounds& bounds,
    double offset
) noexcept {
    double halfH = bounds.height / 2.0;
    double angleRad = (bounds.rotation * std::numbers::pi) / 180.0;
    double cosVal = std::cos(angleRad);
    double sinVal = std::sin(angleRad);

    double localX = 0.0;
    double localY = -(halfH + offset);

    return Vec2D{
        bounds.cx + (localX * cosVal - localY * sinVal),
        bounds.cy + (localX * sinVal + localY * cosVal)
    };
}

static double resolveParamOrAnim(
    const editor::Clip& clip,
    const std::string& key,
    double defaultVal,
    core::TimelineTime localTime
) {
    double base = defaultVal;
    const auto& p = clip.params();
    if (p.contains(key) && p[key].is_number()) {
        base = p[key].get<double>();
    }

    const auto* ch = clip.findAnimationChannel(key);
    if (ch && !ch->empty()) {
        return ch->getValueAt(localTime);
    }
    return base;
}

std::optional<editor::ElementBounds> ElementBoundsEngine::computeVisualBounds(
    const editor::Clip& clip,
    double canvasWidth,
    double canvasHeight,
    core::TimelineTime localTime,
    std::optional<Size2D> mediaSize
) {
    if (clip.type() == editor::ClipType::Audio || clip.type() == editor::ClipType::Effect) {
        return std::nullopt;
    }
    if (clip.isHidden()) {
        return std::nullopt;
    }

    double scaleX = resolveParamOrAnim(clip, "scaleX", 1.0, localTime);
    double scaleY = resolveParamOrAnim(clip, "scaleY", 1.0, localTime);
    // Support unified scale param if present
    if (clip.params().contains("scale") && clip.params()["scale"].is_number()) {
        double s = clip.params()["scale"].get<double>();
        scaleX *= s;
        scaleY *= s;
    }

    double posX = resolveParamOrAnim(clip, "x", 0.0, localTime);
    double posY = resolveParamOrAnim(clip, "y", 0.0, localTime);
    double rot = resolveParamOrAnim(clip, "rotation", 0.0, localTime);

    double sourceWidth = canvasWidth;
    double sourceHeight = canvasHeight;

    if (clip.type() == editor::ClipType::Video || clip.type() == editor::ClipType::Image) {
        if (mediaSize.has_value() && mediaSize->width > 0.0 && mediaSize->height > 0.0) {
            sourceWidth = mediaSize->width;
            sourceHeight = mediaSize->height;
        } else if (clip.params().contains("width") && clip.params().contains("height") &&
                   clip.params()["width"].is_number() && clip.params()["height"].is_number()) {
            sourceWidth = clip.params()["width"].get<double>();
            sourceHeight = clip.params()["height"].get<double>();
        }
    } else if (clip.type() == editor::ClipType::Sticker) {
        sourceWidth = kStickerIntrinsicFallback;
        sourceHeight = kStickerIntrinsicFallback;
        if (clip.params().contains("intrinsicWidth") && clip.params()["intrinsicWidth"].is_number()) {
            sourceWidth = clip.params()["intrinsicWidth"].get<double>();
        }
        if (clip.params().contains("intrinsicHeight") && clip.params()["intrinsicHeight"].is_number()) {
            sourceHeight = clip.params()["intrinsicHeight"].get<double>();
        }
    } else if (clip.type() == editor::ClipType::Graphic) {
        sourceWidth = kDefaultGraphicSourceSize;
        sourceHeight = kDefaultGraphicSourceSize;
        if (clip.params().contains("width") && clip.params()["width"].is_number()) {
            sourceWidth = clip.params()["width"].get<double>();
        }
        if (clip.params().contains("height") && clip.params()["height"].is_number()) {
            sourceHeight = clip.params()["height"].get<double>();
        }
    } else if (clip.type() == editor::ClipType::Text) {
        double textW = 300.0;
        double textH = 100.0;
        if (clip.params().contains("width") && clip.params()["width"].is_number()) {
            textW = clip.params()["width"].get<double>();
        }
        if (clip.params().contains("height") && clip.params()["height"].is_number()) {
            textH = clip.params()["height"].get<double>();
        }
        return editor::ElementBounds{
            canvasWidth / 2.0 + posX,
            canvasHeight / 2.0 + posY,
            std::abs(textW * scaleX),
            std::abs(textH * scaleY),
            rot
        };
    }

    double containScale = std::min(canvasWidth / sourceWidth, canvasHeight / sourceHeight);
    double scaledWidth = sourceWidth * containScale * scaleX;
    double scaledHeight = sourceHeight * containScale * scaleY;

    return editor::ElementBounds{
        canvasWidth / 2.0 + posX,
        canvasHeight / 2.0 + posY,
        std::abs(scaledWidth),
        std::abs(scaledHeight),
        rot
    };
}

std::vector<ElementWithBounds> ElementBoundsEngine::getVisibleElementsWithBounds(
    const std::vector<editor::Track>& tracks,
    core::TimelineTime currentTime,
    double canvasWidth,
    double canvasHeight,
    const std::unordered_map<std::string, Size2D>& mediaSizes
) {
    std::vector<ElementWithBounds> result;

    // Track order: from highest visual layer to lowest (overlay top-down, then main)
    for (auto it = tracks.rbegin(); it != tracks.rend(); ++it) {
        const auto& track = *it;
        if (track.isHidden()) {
            continue;
        }

        std::vector<const editor::Clip*> visibleClips;
        for (const auto& clip : track.clips()) {
            if (clip.isHidden()) continue;
            if (currentTime >= clip.startTime() && currentTime < clip.endTime()) {
                visibleClips.push_back(&clip);
            }
        }

        std::sort(visibleClips.begin(), visibleClips.end(), [](const editor::Clip* a, const editor::Clip* b) {
            if (a->startTime() != b->startTime()) {
                return a->startTime() < b->startTime();
            }
            return a->id().str() < b->id().str();
        });

        for (const auto* clip : visibleClips) {
            core::TimelineTime localTime = currentTime - clip->startTime();
            std::optional<Size2D> mSize = std::nullopt;
            if (!clip->mediaId().isEmpty()) {
                auto mIt = mediaSizes.find(clip->mediaId().str());
                if (mIt != mediaSizes.end()) {
                    mSize = mIt->second;
                }
            }

            auto bounds = computeVisualBounds(*clip, canvasWidth, canvasHeight, localTime, mSize);
            if (bounds.has_value()) {
                result.push_back(ElementWithBounds{
                    track.id().str(),
                    clip->id().str(),
                    clip->type(),
                    *bounds
                });
            }
        }
    }

    return result;
}

bool ElementBoundsEngine::pointInRotatedRect(
    double px,
    double py,
    const editor::ElementBounds& bounds
) noexcept {
    double angleRad = (bounds.rotation * std::numbers::pi) / 180.0;
    double cosVal = std::cos(-angleRad);
    double sinVal = std::sin(-angleRad);

    double dx = px - bounds.cx;
    double dy = py - bounds.cy;

    double localX = dx * cosVal - dy * sinVal;
    double localY = dx * sinVal + dy * cosVal;

    double halfW = std::abs(bounds.width) / 2.0;
    double halfH = std::abs(bounds.height) / 2.0;

    return (localX >= -halfW && localX <= halfW &&
            localY >= -halfH && localY <= halfH);
}

std::vector<ElementWithBounds> ElementBoundsEngine::getHitElements(
    double canvasX,
    double canvasY,
    const std::vector<ElementWithBounds>& elementsWithBounds
) {
    std::vector<ElementWithBounds> hits;
    for (const auto& item : elementsWithBounds) {
        if (pointInRotatedRect(canvasX, canvasY, item.bounds)) {
            hits.push_back(item);
        }
    }
    return hits;
}

std::optional<ElementWithBounds> ElementBoundsEngine::hitTest(
    double canvasX,
    double canvasY,
    const std::vector<ElementWithBounds>& elementsWithBounds
) {
    auto hits = getHitElements(canvasX, canvasY, elementsWithBounds);
    if (!hits.empty()) {
        return hits.front();
    }
    return std::nullopt;
}

std::optional<ElementWithBounds> ElementBoundsEngine::resolvePreferredHit(
    const std::vector<ElementWithBounds>& hits,
    const std::vector<std::string>& preferredElementIds
) {
    if (hits.empty()) {
        return std::nullopt;
    }

    if (preferredElementIds.empty()) {
        return hits.front();
    }

    for (const auto& hit : hits) {
        for (const auto& prefId : preferredElementIds) {
            if (hit.elementId == prefId) {
                return hit;
            }
        }
    }

    return hits.front();
}

} // namespace catchim::render
