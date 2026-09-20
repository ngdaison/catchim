#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include "editor/timeline/ElementUtils.h"
#include <vector>
#include <memory>
#include <string>

namespace catchim::editor {

/**
 * @brief Command to duplicate selected timeline elements into new tracks,
 * preserving all clip parameters, media IDs, and animation channels.
 * Corresponds to web/src/commands/timeline/element/duplicate-elements.ts.
 */
class DuplicateElementsCommand : public EditorCommand {
public:
    DuplicateElementsCommand(Timeline& timeline, std::vector<ElementLocation> elements);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return "Duplicate Elements"; }

    const std::vector<ElementLocation>& duplicatedElements() const noexcept { return duplicatedElements_; }

private:
    Timeline& timeline_;
    std::vector<ElementLocation> elementsToDuplicate_;
    std::vector<ElementLocation> duplicatedElements_;
    std::optional<TimelineTracksSnapshot> savedSnapshot_{std::nullopt};
};

} // namespace catchim::editor
