#include "TimelineDragDropController.h"

namespace catchim::editor {

void TimelineDragDropController::onDragEnter(ClipType type) {
    state_ = DragDropState{
        .kind = DragDropStateKind::Over,
        .dropTarget = std::nullopt,
        .elementType = type
    };
}

void TimelineDragDropController::onDragOver(
    const Timeline& timeline,
    double mouseX,
    double mouseY,
    ClipType type,
    core::TimelineTime duration,
    const std::vector<ClipType>& targetElementTypes
) {
    ComputeDropTargetParams params{
        .clipType = type,
        .mouseX = mouseX,
        .mouseY = mouseY,
        .playheadTime = config_.playheadTime,
        .isExternalDrop = false,
        .elementDuration = duration,
        .pixelsPerSecond = 50.0,
        .zoomLevel = config_.zoomLevel,
        .verticalDragDirection = std::nullopt,
        .startTimeOverride = std::nullopt,
        .targetElementTypes = targetElementTypes
    };

    auto target = TimelineDropTargetResolver::computeDropTarget(timeline, params);
    state_ = DragDropState{
        .kind = DragDropStateKind::Over,
        .dropTarget = target,
        .elementType = type
    };
}

void TimelineDragDropController::onDragLeave() noexcept {
    state_ = DragDropState{
        .kind = DragDropStateKind::Idle,
        .dropTarget = std::nullopt,
        .elementType = std::nullopt
    };
}

bool TimelineDragDropController::onDrop(
    const Timeline& timeline,
    double mouseX,
    double mouseY,
    ClipType type,
    core::TimelineTime duration,
    const std::string& effectType
) {
    ComputeDropTargetParams params{
        .clipType = type,
        .mouseX = mouseX,
        .mouseY = mouseY,
        .playheadTime = config_.playheadTime,
        .isExternalDrop = false,
        .elementDuration = duration,
        .pixelsPerSecond = 50.0,
        .zoomLevel = config_.zoomLevel,
        .verticalDragDirection = std::nullopt,
        .startTimeOverride = std::nullopt,
        .targetElementTypes = effectType.empty() ? std::vector<ClipType>{} : std::vector<ClipType>{ClipType::Video, ClipType::Image}
    };

    auto target = TimelineDropTargetResolver::computeDropTarget(timeline, params);

    if (target.targetElement.has_value() && !effectType.empty()) {
        if (config_.onAddClipEffect) {
            config_.onAddClipEffect(*target.targetElement, effectType);
        }
    } else {
        if (config_.onDropElement) {
            config_.onDropElement(target, type);
        }
    }

    onDragLeave();
    return true;
}

} // namespace catchim::editor
