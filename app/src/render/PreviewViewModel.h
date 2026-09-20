#pragma once

#include "editor/EditorEngine.h"
#include "render/HitTesting.h"
#include "render/PreviewSnap.h"
#include "render/SafeZoneGuide.h"
#include <vector>
#include <optional>

namespace catchim::render {

enum class DragMode {
    None,
    Move,
    Scale,
    Rotate
};

struct CanvasGizmoHandle {
    GizmoHandle handle{GizmoHandle::None};
    Point2D screenPos;
    double size{8.0};
};

class PreviewViewModel {
public:
    explicit PreviewViewModel(editor::EditorEngine& engine);

    editor::EditorEngine& getEngine() { return engine_; }
    const editor::EditorEngine& getEngine() const { return engine_; }

    // Viewport & Zoom
    void setViewportSize(double width, double height);
    double getViewportWidth() const { return viewportWidth_; }
    double getViewportHeight() const { return viewportHeight_; }

    void setZoom(double zoom);
    double getZoom() const { return zoom_; }
    void zoomToFit();

    void setPanOffset(double panX, double panY);
    Point2D getPanOffset() const { return {panOffsetX_, panOffsetY_}; }

    // Coordinate conversions (Canvas centered at 0,0)
    Point2D canvasToScreen(Point2D canvasPoint) const;
    Point2D screenToCanvas(Point2D screenPoint) const;
    Size2D getCanvasSize() const;

    // Overlays & Safe zones
    void setSafeZoneType(std::optional<GuideType> type) { safeZoneType_ = type; }
    std::optional<GuideType> getSafeZoneType() const { return safeZoneType_; }

    void setGridEnabled(bool enabled) { gridEnabled_ = enabled; }
    bool isGridEnabled() const { return gridEnabled_; }

    void setSnappingEnabled(bool enabled) { snappingEnabled_ = enabled; }
    bool isSnappingEnabled() const { return snappingEnabled_; }

    const std::vector<SnapLine>& getActiveSnapLines() const { return activeSnapLines_; }
    void clearActiveSnapLines() { activeSnapLines_.clear(); }

    // Gizmo handles for currently selected element
    std::vector<CanvasGizmoHandle> getGizmoHandles() const;
    std::optional<GizmoHandle> hitTestHandle(Point2D screenPoint, double tolerance = 6.0) const;

    // Interactive transforms
    void startDrag(DragMode mode, Point2D startScreenPoint, GizmoHandle handle = GizmoHandle::TopLeft);
    void updateDrag(Point2D currentScreenPoint);
    void endDrag();
    void cancelDrag();
    DragMode getDragMode() const { return dragMode_; }

private:
    editor::EditorEngine& engine_;
    double viewportWidth_{800.0};
    double viewportHeight_{600.0};
    double zoom_{1.0};
    double panOffsetX_{0.0};
    double panOffsetY_{0.0};

    std::optional<GuideType> safeZoneType_{std::nullopt};
    bool gridEnabled_{false};
    bool snappingEnabled_{true};
    std::vector<SnapLine> activeSnapLines_;

    DragMode dragMode_{DragMode::None};
    GizmoHandle activeHandle_{GizmoHandle::TopLeft};
    Point2D dragStartScreenPoint_{0.0, 0.0};
    Point2D initialElementPos_{0.0, 0.0};
    double initialElementScale_{1.0};
    double initialElementRotation_{0.0};
    core::ClipId activeClipId_{core::ClipId::empty()};
};

} // namespace catchim::render
