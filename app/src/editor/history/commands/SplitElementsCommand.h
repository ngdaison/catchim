#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/ElementUtils.h"
#include <vector>
#include <string>
#include <optional>

namespace catchim::editor {

enum class RetainSide {
    Both,
    Left,
    Right
};

/**
 * @brief Command to atomically split multiple timeline elements across multiple tracks.
 * Corresponds to web/src/commands/timeline/element/split-elements.ts.
 */
class SplitElementsCommand : public EditorCommand {
public:
    SplitElementsCommand(
        Timeline& timeline,
        std::vector<ElementLocation> elements,
        core::TimelineTime splitTime,
        RetainSide retainSide = RetainSide::Both
    );

    bool execute() override;
    bool undo() override;
    [[nodiscard]] std::string name() const override;

    const std::vector<ElementLocation>& elements() const noexcept { return elements_; }
    core::TimelineTime splitTime() const noexcept { return splitTime_; }
    RetainSide retainSide() const noexcept { return retainSide_; }
    const std::vector<ElementLocation>& rightSideElements() const noexcept { return rightSideElements_; }

private:
    Timeline& timeline_;
    std::vector<ElementLocation> elements_;
    core::TimelineTime splitTime_;
    RetainSide retainSide_;
    std::vector<ElementLocation> rightSideElements_;
    std::optional<TimelineTracksSnapshot> savedState_{std::nullopt};
};

} // namespace catchim::editor
