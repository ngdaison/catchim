#include "PreviewInteractionGestureController.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

bool PreviewInteractionGestureController::movedPastDragThreshold(
    const PreviewPoint& current,
    const PreviewPoint& origin,
    double threshold
) noexcept {
    return std::abs(current.x - origin.x) > threshold ||
           std::abs(current.y - origin.y) > threshold;
}

std::vector<ElementRef> PreviewInteractionGestureController::buildDragSelection(
    const std::vector<ElementRef>& selectedElements,
    const ElementRef& dragTarget
) {
    auto it = std::find(selectedElements.begin(), selectedElements.end(), dragTarget);
    if (it == selectedElements.end()) {
        return {dragTarget};
    }

    std::vector<ElementRef> result;
    result.reserve(selectedElements.size());
    result.push_back(dragTarget);
    for (const auto& el : selectedElements) {
        if (!(el == dragTarget)) {
            result.push_back(el);
        }
    }
    return result;
}

void PreviewInteractionGestureController::startPending(
    const PreviewPoint& origin,
    const std::vector<ElementRef>& selected
) {
    kind_ = PreviewInteractionGestureKind::Pending;
    origin_ = origin;
    current_ = origin;
    activeSelection_ = selected;
}

bool PreviewInteractionGestureController::handleMove(
    const PreviewPoint& current,
    double threshold
) {
    current_ = current;
    if (kind_ == PreviewInteractionGestureKind::Pending) {
        if (movedPastDragThreshold(current_, origin_, threshold)) {
            kind_ = PreviewInteractionGestureKind::Dragging;
            return true;
        }
    }
    return kind_ == PreviewInteractionGestureKind::Dragging;
}

void PreviewInteractionGestureController::endGesture() noexcept {
    kind_ = PreviewInteractionGestureKind::Idle;
    activeSelection_.clear();
}

void PreviewInteractionGestureController::cancelGesture() noexcept {
    kind_ = PreviewInteractionGestureKind::Idle;
    activeSelection_.clear();
}

void PreviewInteractionGestureController::startTextEdit(
    core::TrackId trackId,
    core::ClipId elementId,
    std::string initialText
) {
    editingTextState_ = EditingTextState{
        .trackId = trackId,
        .elementId = elementId,
        .text = std::move(initialText)
    };
}

void PreviewInteractionGestureController::commitTextEdit(std::string finalContent) {
    if (editingTextState_.has_value()) {
        editingTextState_->text = std::move(finalContent);
    }
    editingTextState_ = std::nullopt;
}

void PreviewInteractionGestureController::cancelTextEdit() noexcept {
    editingTextState_ = std::nullopt;
}

} // namespace catchim::editor
