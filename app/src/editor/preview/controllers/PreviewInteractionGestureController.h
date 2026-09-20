#pragma once

#include "editor/selection/EditorSelection.h"
#include <vector>
#include <string>
#include <optional>

namespace catchim::editor {

struct PreviewPoint {
    double x{0.0};
    double y{0.0};

    bool operator==(const PreviewPoint& other) const = default;
};

enum class PreviewInteractionGestureKind {
    Idle,
    Pending,
    Dragging
};

struct EditingTextState {
    core::TrackId trackId{core::TrackId::empty()};
    core::ClipId elementId{core::ClipId::empty()};
    std::string text;

    bool operator==(const EditingTextState& other) const = default;
};

class PreviewInteractionGestureController {
public:
    static constexpr double MIN_DRAG_DISTANCE = 0.5;

    PreviewInteractionGestureController() = default;

    PreviewInteractionGestureKind gestureKind() const noexcept { return kind_; }
    bool isDragging() const noexcept { return kind_ == PreviewInteractionGestureKind::Dragging; }
    bool isPending() const noexcept { return kind_ == PreviewInteractionGestureKind::Pending; }
    bool isIdle() const noexcept { return kind_ == PreviewInteractionGestureKind::Idle; }

    static bool movedPastDragThreshold(
        const PreviewPoint& current,
        const PreviewPoint& origin,
        double threshold = MIN_DRAG_DISTANCE
    ) noexcept;

    static std::vector<ElementRef> buildDragSelection(
        const std::vector<ElementRef>& selectedElements,
        const ElementRef& dragTarget
    );

    void startPending(const PreviewPoint& origin, const std::vector<ElementRef>& selected);
    bool handleMove(const PreviewPoint& current, double threshold = MIN_DRAG_DISTANCE);
    void endGesture() noexcept;
    void cancelGesture() noexcept;

    // Direct Canvas Text Editing
    bool isEditingText() const noexcept { return editingTextState_.has_value(); }
    const std::optional<EditingTextState>& editingTextState() const noexcept { return editingTextState_; }
    void startTextEdit(core::TrackId trackId, core::ClipId elementId, std::string initialText);
    void commitTextEdit(std::string finalContent);
    void cancelTextEdit() noexcept;

private:
    PreviewInteractionGestureKind kind_{PreviewInteractionGestureKind::Idle};
    PreviewPoint origin_{0.0, 0.0};
    PreviewPoint current_{0.0, 0.0};
    std::vector<ElementRef> activeSelection_;
    std::optional<EditingTextState> editingTextState_{std::nullopt};
};

} // namespace catchim::editor
