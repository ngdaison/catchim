#pragma once

#include "TimelineDropTargetResolver.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/Clip.h"
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace catchim::editor {

enum class DragDropStateKind {
    Idle,
    Over
};

struct DragDropState {
    DragDropStateKind kind{DragDropStateKind::Idle};
    std::optional<DropTarget> dropTarget{std::nullopt};
    std::optional<ClipType> elementType{std::nullopt};
};

struct DragDropConfig {
    double zoomLevel{1.0};
    core::TimelineTime playheadTime{0};
    std::function<void(const DropTarget&, ClipType)> onDropElement;
    std::function<void(const DropTargetElement&, const std::string&)> onAddClipEffect;
};

class TimelineDragDropController {
public:
    explicit TimelineDragDropController(DragDropConfig config = {}) : config_(std::move(config)) {}

    void setConfig(DragDropConfig config) { config_ = std::move(config); }
    [[nodiscard]] const DragDropConfig& config() const noexcept { return config_; }

    [[nodiscard]] bool isOver() const noexcept {
        return state_.kind == DragDropStateKind::Over;
    }

    [[nodiscard]] const DragDropState& state() const noexcept { return state_; }

    void onDragEnter(ClipType type);

    void onDragOver(
        const Timeline& timeline,
        double mouseX,
        double mouseY,
        ClipType type,
        core::TimelineTime duration = core::TimelineTime::fromSeconds(5.0),
        const std::vector<ClipType>& targetElementTypes = {}
    );

    void onDragLeave() noexcept;

    bool onDrop(
        const Timeline& timeline,
        double mouseX,
        double mouseY,
        ClipType type,
        core::TimelineTime duration = core::TimelineTime::fromSeconds(5.0),
        const std::string& effectType = ""
    );

private:
    DragDropConfig config_;
    DragDropState state_;
};

} // namespace catchim::editor
