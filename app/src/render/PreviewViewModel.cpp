#include "render/PreviewViewModel.h"
#include "render/Transform.h"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::render {

namespace {

Transform getClipTransform(const editor::Clip& clip) {
    Transform t;
    t.positionX = clip.getParam<double>("transform.positionX", 0.0);
    t.positionY = clip.getParam<double>("transform.positionY", 0.0);
    t.scaleX = clip.getParam<double>("transform.scaleX", 1.0);
    t.scaleY = clip.getParam<double>("transform.scaleY", 1.0);
    t.rotate = clip.getParam<double>("transform.rotate", 0.0);
    t.opacity = clip.getParam<double>("opacity", 1.0);
    return t;
}

void setClipTransform(editor::Clip& clip, const Transform& t) {
    clip.setParam("transform.positionX", t.positionX);
    clip.setParam("transform.positionY", t.positionY);
    clip.setParam("transform.scaleX", t.scaleX);
    clip.setParam("transform.scaleY", t.scaleY);
    clip.setParam("transform.rotate", t.rotate);
    clip.setParam("opacity", t.opacity);
}

} // namespace

PreviewViewModel::PreviewViewModel(editor::EditorEngine& engine)
    : engine_(engine)
{
    zoomToFit();
}

void PreviewViewModel::setViewportSize(double width, double height) {
    if (width > 10.0 && height > 10.0) {
        viewportWidth_ = width;
        viewportHeight_ = height;
    }
}

void PreviewViewModel::setZoom(double zoom) {
    zoom_ = std::clamp(zoom, 0.05, 10.0);
}

void PreviewViewModel::setPanOffset(double panX, double panY) {
    panOffsetX_ = panX;
    panOffsetY_ = panY;
}

Size2D PreviewViewModel::getCanvasSize() const {
    const auto& settings = engine_.project().settings();
    return {
        static_cast<double>(settings.canvasSize.width),
        static_cast<double>(settings.canvasSize.height)
    };
}

void PreviewViewModel::zoomToFit() {
    Size2D canvas = getCanvasSize();
    if (canvas.width <= 0.0 || canvas.height <= 0.0) return;

    constexpr double margin = 20.0;
    double availW = std::max(10.0, viewportWidth_ - 2.0 * margin);
    double availH = std::max(10.0, viewportHeight_ - 2.0 * margin);

    double scaleX = availW / canvas.width;
    double scaleY = availH / canvas.height;
    zoom_ = std::min(scaleX, scaleY);
    panOffsetX_ = 0.0;
    panOffsetY_ = 0.0;
}

Point2D PreviewViewModel::canvasToScreen(Point2D canvasPoint) const {
    double screenCenterX = viewportWidth_ / 2.0 + panOffsetX_;
    double screenCenterY = viewportHeight_ / 2.0 + panOffsetY_;
    return {
        screenCenterX + canvasPoint.x * zoom_,
        screenCenterY + canvasPoint.y * zoom_
    };
}

Point2D PreviewViewModel::screenToCanvas(Point2D screenPoint) const {
    double screenCenterX = viewportWidth_ / 2.0 + panOffsetX_;
    double screenCenterY = viewportHeight_ / 2.0 + panOffsetY_;
    return {
        (screenPoint.x - screenCenterX) / zoom_,
        (screenPoint.y - screenCenterY) / zoom_
    };
}

std::vector<CanvasGizmoHandle> PreviewViewModel::getGizmoHandles() const {
    std::vector<CanvasGizmoHandle> handles;
    const auto& selected = engine_.selectedClips();
    if (selected.size() != 1) return handles;

    const auto* tl = engine_.activeTimeline();
    if (!tl) return handles;

    const auto* clip = tl->findClip(selected[0]);
    if (!clip) return handles;

    Transform t = getClipTransform(*clip);
    double baseW = 200.0;
    double baseH = 200.0;
    double halfW = (baseW * t.scaleX) / 2.0;
    double halfH = (baseH * t.scaleY) / 2.0;

    double rotRad = (t.rotate * M_PI) / 180.0;
    double cosR = std::cos(rotRad);
    double sinR = std::sin(rotRad);

    auto transformLocal = [&](double lx, double ly) -> Point2D {
        double rx = lx * cosR - ly * sinR + t.positionX;
        double ry = lx * sinR + ly * cosR + t.positionY;
        return canvasToScreen({rx, ry});
    };

    handles.push_back({GizmoHandle::TopLeft, transformLocal(-halfW, -halfH)});
    handles.push_back({GizmoHandle::Top, transformLocal(0.0, -halfH)});
    handles.push_back({GizmoHandle::TopRight, transformLocal(halfW, -halfH)});
    handles.push_back({GizmoHandle::Left, transformLocal(-halfW, 0.0)});
    handles.push_back({GizmoHandle::Right, transformLocal(halfW, 0.0)});
    handles.push_back({GizmoHandle::BottomLeft, transformLocal(-halfW, halfH)});
    handles.push_back({GizmoHandle::Bottom, transformLocal(0.0, halfH)});
    handles.push_back({GizmoHandle::BottomRight, transformLocal(halfW, halfH)});

    // Rotation handle positioned 24px above top center in local element space
    handles.push_back({GizmoHandle::Rotate, transformLocal(0.0, -halfH - 24.0 / zoom_)});

    return handles;
}

std::optional<GizmoHandle> PreviewViewModel::hitTestHandle(Point2D screenPoint, double tolerance) const {
    auto handles = getGizmoHandles();
    for (const auto& h : handles) {
        double dx = screenPoint.x - h.screenPos.x;
        double dy = screenPoint.y - h.screenPos.y;
        if (std::sqrt(dx * dx + dy * dy) <= tolerance) {
            return h.handle;
        }
    }
    return std::nullopt;
}

void PreviewViewModel::startDrag(DragMode mode, Point2D startScreenPoint, GizmoHandle handle) {
    dragMode_ = mode;
    activeHandle_ = handle;
    dragStartScreenPoint_ = startScreenPoint;
    activeSnapLines_.clear();

    const auto& selected = engine_.selectedClips();
    if (selected.size() == 1) {
        activeClipId_ = selected[0];
        const auto* tl = engine_.activeTimeline();
        const auto* clip = tl ? tl->findClip(activeClipId_) : nullptr;
        if (clip) {
            Transform t = getClipTransform(*clip);
            initialElementPos_ = {t.positionX, t.positionY};
            initialElementScale_ = t.scaleX;
            initialElementRotation_ = t.rotate;
        }
    } else {
        activeClipId_ = core::ClipId::empty();
    }
}

void PreviewViewModel::updateDrag(Point2D currentScreenPoint) {
    if (dragMode_ == DragMode::None || activeClipId_.isEmpty()) return;

    auto* tl = engine_.activeTimeline();
    if (!tl) return;
    auto* clip = tl->findClip(activeClipId_);
    if (!clip) return;

    Point2D startCanvas = screenToCanvas(dragStartScreenPoint_);
    Point2D currCanvas = screenToCanvas(currentScreenPoint);
    Point2D delta = {currCanvas.x - startCanvas.x, currCanvas.y - startCanvas.y};

    Transform t = getClipTransform(*clip);
    Size2D canvasSize = getCanvasSize();

    if (dragMode_ == DragMode::Move) {
        Point2D proposed = {initialElementPos_.x + delta.x, initialElementPos_.y + delta.y};
        if (snappingEnabled_) {
            Size2D elemSize = {200.0 * initialElementScale_, 200.0 * initialElementScale_};
            PreviewSnapResult snap = PreviewSnap::snapPosition(proposed, canvasSize, elemSize, t.rotate);
            t.positionX = snap.snappedPosition.x;
            t.positionY = snap.snappedPosition.y;
            activeSnapLines_ = snap.activeLines;
        } else {
            t.positionX = proposed.x;
            t.positionY = proposed.y;
            activeSnapLines_.clear();
        }
        setClipTransform(*clip, t);
    } else if (dragMode_ == DragMode::Rotate) {
        Point2D elemCenterScreen = canvasToScreen({t.positionX, t.positionY});
        double angleRad = std::atan2(currentScreenPoint.y - elemCenterScreen.y, currentScreenPoint.x - elemCenterScreen.x);
        double angleDeg = angleRad * 180.0 / M_PI + 90.0;

        if (snappingEnabled_) {
            RotationSnapResult snap = PreviewSnap::snapRotation(angleDeg);
            t.rotate = snap.snappedRotation;
        } else {
            t.rotate = angleDeg;
        }
        setClipTransform(*clip, t);
    } else if (dragMode_ == DragMode::Scale) {
        double distDelta = (currentScreenPoint.x - dragStartScreenPoint_.x) / 100.0;
        double proposedScale = std::max(0.05, initialElementScale_ + distDelta);

        if (snappingEnabled_) {
            ScaleSnapResult snap = PreviewSnap::snapScale(
                proposedScale,
                Point2D{t.positionX, t.positionY},
                200.0,
                200.0,
                t.rotate,
                canvasSize
            );
            t.scaleX = snap.snappedScale;
            t.scaleY = snap.snappedScale;
            activeSnapLines_ = snap.activeLines;
        } else {
            t.scaleX = proposedScale;
            t.scaleY = proposedScale;
            activeSnapLines_.clear();
        }
        setClipTransform(*clip, t);
    }
}

void PreviewViewModel::endDrag() {
    dragMode_ = DragMode::None;
    activeSnapLines_.clear();
}

void PreviewViewModel::cancelDrag() {
    if (!activeClipId_.isEmpty()) {
        auto* tl = engine_.activeTimeline();
        auto* clip = tl ? tl->findClip(activeClipId_) : nullptr;
        if (clip) {
            Transform t = getClipTransform(*clip);
            t.positionX = initialElementPos_.x;
            t.positionY = initialElementPos_.y;
            t.scaleX = initialElementScale_;
            t.scaleY = initialElementScale_;
            t.rotate = initialElementRotation_;
            setClipTransform(*clip, t);
        }
    }
    dragMode_ = DragMode::None;
    activeSnapLines_.clear();
}

} // namespace catchim::render
