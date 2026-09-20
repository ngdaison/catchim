#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include "editor/clipboard/ClipboardKeyframeEngine.h"
#include <string>
#include <vector>
#include <optional>

namespace catchim::editor {

struct ElementRef {
    core::TrackId trackId;
    core::ClipId clipId;

    bool operator==(const ElementRef& other) const = default;
};

struct SelectedMaskPointSelection {
    core::TrackId trackId;
    core::ClipId clipId;
    std::string maskId;
    std::vector<std::string> pointIds;

    bool operator==(const SelectedMaskPointSelection& other) const = default;
};

enum class EditorSelectionKind {
    Elements,
    Keyframes,
    MaskPoints
};

struct EditorSelectionSnapshot {
    std::vector<ElementRef> selectedElements;
    std::vector<SelectedKeyframeRef> selectedKeyframes;
    std::optional<SelectedKeyframeRef> keyframeSelectionAnchor{std::nullopt};
    std::optional<SelectedMaskPointSelection> selectedMaskPoints{std::nullopt};
};

struct EditorSelectionPatch {
    std::optional<std::vector<ElementRef>> selectedElements{std::nullopt};
    std::optional<std::vector<SelectedKeyframeRef>> selectedKeyframes{std::nullopt};
    std::optional<std::optional<SelectedKeyframeRef>> keyframeSelectionAnchor{std::nullopt};
    std::optional<std::optional<SelectedMaskPointSelection>> selectedMaskPoints{std::nullopt};
};

class EditorSelection {
public:
    EditorSelection() = default;

    [[nodiscard]] const std::vector<ElementRef>& getSelectedElements() const noexcept {
        return selectedElements_;
    }

    [[nodiscard]] const std::vector<SelectedKeyframeRef>& getSelectedKeyframes() const noexcept {
        return selectedKeyframes_;
    }

    [[nodiscard]] const std::optional<SelectedKeyframeRef>& getKeyframeSelectionAnchor() const noexcept {
        return keyframeSelectionAnchor_;
    }

    [[nodiscard]] const std::optional<SelectedMaskPointSelection>& getSelectedMaskPointSelection() const noexcept {
        return selectedMaskPoints_;
    }

    [[nodiscard]] std::optional<EditorSelectionKind> getActiveSelectionKind() const noexcept;

    [[nodiscard]] EditorSelectionSnapshot getSnapshot() const;

    void setSelectedElements(std::vector<ElementRef> elements);

    void setSelectedKeyframes(
        std::vector<SelectedKeyframeRef> keyframes,
        std::optional<SelectedKeyframeRef> anchorKeyframe = std::nullopt
    );

    void setSelectedMaskPoints(std::optional<SelectedMaskPointSelection> selection);

    void clearSelection() noexcept;
    void clearKeyframeSelection() noexcept;
    void clearMaskPointSelection() noexcept;

    /**
     * @brief Clears the most specific selection tier (mask-points -> keyframes -> elements).
     * Mirrors web/src/core/managers/selection-manager.ts clearMostSpecificSelection.
     */
    bool clearMostSpecificSelection();

    EditorSelectionSnapshot applySelectionPatch(const EditorSelectionPatch& patch);
    void restoreSnapshot(const EditorSelectionSnapshot& snapshot);

private:
    std::vector<ElementRef> selectedElements_;
    std::vector<SelectedKeyframeRef> selectedKeyframes_;
    std::optional<SelectedKeyframeRef> keyframeSelectionAnchor_{std::nullopt};
    std::optional<SelectedMaskPointSelection> selectedMaskPoints_{std::nullopt};
};

} // namespace catchim::editor
