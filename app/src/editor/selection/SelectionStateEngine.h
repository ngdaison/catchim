#pragma once

#include "SelectionManager.h"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace catchim::editor {

struct BoxSelectionChange {
    std::vector<std::string> intersectedIds;
    std::vector<std::string> initialSelectedIds;
    std::optional<std::string> initialAnchorId{std::nullopt};
    bool isAdditive{false};
};

struct ScopeEntry {
    std::function<bool()> hasSelection;
    std::function<void()> clear;
    std::function<void()> clearActive;
};

class SelectionStateEngine {
public:
    static std::vector<std::string> dedupeIds(const std::vector<std::string>& ids);

    static std::vector<std::string> getRangeIds(
        const std::vector<std::string>& orderedIds,
        const std::string& anchorId,
        const std::string& targetId
    );

    static SelectionState replaceSelection(
        const std::vector<std::string>& ids,
        const std::optional<std::string>& anchorId = std::nullopt
    );

    static SelectionState clearSelection() noexcept;

    static SelectionState pruneSelection(
        const SelectionState& state,
        const std::vector<std::string>& orderedIds
    );

    static bool isSelected(
        const SelectionState& state,
        const std::string& id
    ) noexcept;

    static SelectionState toggleSelection(
        const SelectionState& state,
        const std::string& id
    );

    static SelectionState selectRange(
        const SelectionState& state,
        const std::vector<std::string>& orderedIds,
        const std::string& targetId,
        bool isAdditive
    );

    static SelectionState applyBoxSelection(const BoxSelectionChange& change);

    // Scope management
    static void activateScope(ScopeEntry entry);
    static bool clearActiveScope();
    static bool hasActiveScopeSelection();
    static void resetActiveScope() noexcept;
};

} // namespace catchim::editor
