#include "EditorSelection.h"

namespace catchim::editor {

std::optional<EditorSelectionKind> EditorSelection::getActiveSelectionKind() const noexcept {
    if (selectedMaskPoints_.has_value() && !selectedMaskPoints_->pointIds.empty()) {
        return EditorSelectionKind::MaskPoints;
    }
    if (!selectedKeyframes_.empty()) {
        return EditorSelectionKind::Keyframes;
    }
    if (!selectedElements_.empty()) {
        return EditorSelectionKind::Elements;
    }
    return std::nullopt;
}

EditorSelectionSnapshot EditorSelection::getSnapshot() const {
    return EditorSelectionSnapshot{
        selectedElements_,
        selectedKeyframes_,
        keyframeSelectionAnchor_,
        selectedMaskPoints_
    };
}

void EditorSelection::setSelectedElements(std::vector<ElementRef> elements) {
    selectedElements_ = std::move(elements);
    selectedKeyframes_.clear();
    keyframeSelectionAnchor_.reset();
    selectedMaskPoints_.reset();
}

void EditorSelection::setSelectedKeyframes(
    std::vector<SelectedKeyframeRef> keyframes,
    std::optional<SelectedKeyframeRef> anchorKeyframe
) {
    selectedKeyframes_ = std::move(keyframes);
    if (anchorKeyframe.has_value()) {
        keyframeSelectionAnchor_ = std::move(anchorKeyframe);
    } else if (selectedKeyframes_.empty()) {
        keyframeSelectionAnchor_.reset();
    }
    selectedMaskPoints_.reset();
}

void EditorSelection::setSelectedMaskPoints(std::optional<SelectedMaskPointSelection> selection) {
    if (selection.has_value() && !selection->pointIds.empty()) {
        selectedMaskPoints_ = std::move(selection);
    } else {
        selectedMaskPoints_.reset();
    }
    selectedKeyframes_.clear();
    keyframeSelectionAnchor_.reset();
}

void EditorSelection::clearSelection() noexcept {
    selectedElements_.clear();
    selectedKeyframes_.clear();
    keyframeSelectionAnchor_.reset();
    selectedMaskPoints_.reset();
}

void EditorSelection::clearKeyframeSelection() noexcept {
    selectedKeyframes_.clear();
    keyframeSelectionAnchor_.reset();
}

void EditorSelection::clearMaskPointSelection() noexcept {
    selectedMaskPoints_.reset();
}

bool EditorSelection::clearMostSpecificSelection() {
    auto kind = getActiveSelectionKind();
    if (!kind.has_value()) {
        return false;
    }
    if (*kind == EditorSelectionKind::MaskPoints) {
        clearMaskPointSelection();
        return true;
    }
    if (*kind == EditorSelectionKind::Keyframes) {
        clearKeyframeSelection();
        return true;
    }
    if (*kind == EditorSelectionKind::Elements) {
        selectedElements_.clear();
        return true;
    }
    return false;
}

EditorSelectionSnapshot EditorSelection::applySelectionPatch(const EditorSelectionPatch& patch) {
    if (patch.selectedElements.has_value()) {
        selectedElements_ = *patch.selectedElements;
    }
    if (patch.selectedKeyframes.has_value()) {
        selectedKeyframes_ = *patch.selectedKeyframes;
    }
    if (patch.keyframeSelectionAnchor.has_value()) {
        keyframeSelectionAnchor_ = *patch.keyframeSelectionAnchor;
    }
    if (patch.selectedMaskPoints.has_value()) {
        selectedMaskPoints_ = *patch.selectedMaskPoints;
    }
    return getSnapshot();
}

void EditorSelection::restoreSnapshot(const EditorSelectionSnapshot& snapshot) {
    selectedElements_ = snapshot.selectedElements;
    selectedKeyframes_ = snapshot.selectedKeyframes;
    keyframeSelectionAnchor_ = snapshot.keyframeSelectionAnchor;
    selectedMaskPoints_ = snapshot.selectedMaskPoints;
}

} // namespace catchim::editor
