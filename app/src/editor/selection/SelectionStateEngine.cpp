#include "SelectionStateEngine.h"
#include <algorithm>
#include <unordered_set>

namespace catchim::editor {

namespace {
std::optional<ScopeEntry> s_activeScope{std::nullopt};
}

std::vector<std::string> SelectionStateEngine::dedupeIds(const std::vector<std::string>& ids) {
    std::vector<std::string> result;
    std::unordered_set<std::string> seen;
    result.reserve(ids.size());

    for (const auto& id : ids) {
        if (seen.insert(id).second) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<std::string> SelectionStateEngine::getRangeIds(
    const std::vector<std::string>& orderedIds,
    const std::string& anchorId,
    const std::string& targetId
) {
    const auto itAnchor = std::find(orderedIds.begin(), orderedIds.end(), anchorId);
    const auto itTarget = std::find(orderedIds.begin(), orderedIds.end(), targetId);

    if (itAnchor == orderedIds.end() || itTarget == orderedIds.end()) {
        return {targetId};
    }

    const auto anchorIdx = std::distance(orderedIds.begin(), itAnchor);
    const auto targetIdx = std::distance(orderedIds.begin(), itTarget);

    const auto rangeStart = std::min(anchorIdx, targetIdx);
    const auto rangeEnd = std::max(anchorIdx, targetIdx);

    return std::vector<std::string>(
        orderedIds.begin() + rangeStart,
        orderedIds.begin() + rangeEnd + 1
    );
}

SelectionState SelectionStateEngine::replaceSelection(
    const std::vector<std::string>& ids,
    const std::optional<std::string>& anchorId
) {
    auto selectedIds = dedupeIds(ids);

    std::optional<std::string> resolvedAnchor = anchorId;
    if (!resolvedAnchor.has_value() && !selectedIds.empty()) {
        resolvedAnchor = selectedIds.back();
    }

    return SelectionState{
        .selectedIds = std::move(selectedIds),
        .anchorId = resolvedAnchor
    };
}

SelectionState SelectionStateEngine::clearSelection() noexcept {
    return SelectionState{
        .selectedIds = {},
        .anchorId = std::nullopt
    };
}

SelectionState SelectionStateEngine::pruneSelection(
    const SelectionState& state,
    const std::vector<std::string>& orderedIds
) {
    const std::unordered_set<std::string> validIds(orderedIds.begin(), orderedIds.end());
    std::vector<std::string> selectedIds;

    for (const auto& id : state.selectedIds) {
        if (validIds.contains(id)) {
            selectedIds.push_back(id);
        }
    }

    std::optional<std::string> anchorId = std::nullopt;
    if (state.anchorId.has_value() && validIds.contains(*state.anchorId)) {
        anchorId = state.anchorId;
    } else if (!selectedIds.empty()) {
        anchorId = selectedIds.back();
    }

    return SelectionState{
        .selectedIds = std::move(selectedIds),
        .anchorId = anchorId
    };
}

bool SelectionStateEngine::isSelected(
    const SelectionState& state,
    const std::string& id
) noexcept {
    return std::find(state.selectedIds.begin(), state.selectedIds.end(), id) != state.selectedIds.end();
}

SelectionState SelectionStateEngine::toggleSelection(
    const SelectionState& state,
    const std::string& id
) {
    if (isSelected(state, id)) {
        std::vector<std::string> selectedIds;
        selectedIds.reserve(state.selectedIds.size());

        for (const auto& item : state.selectedIds) {
            if (item != id) {
                selectedIds.push_back(item);
            }
        }

        std::optional<std::string> anchorId = state.anchorId;
        if (anchorId.has_value() && *anchorId == id) {
            anchorId = selectedIds.empty() ? std::nullopt : std::optional<std::string>(selectedIds.back());
        }

        return replaceSelection(selectedIds, anchorId);
    }

    auto selectedIds = state.selectedIds;
    selectedIds.push_back(id);
    return replaceSelection(selectedIds, id);
}

SelectionState SelectionStateEngine::selectRange(
    const SelectionState& state,
    const std::vector<std::string>& orderedIds,
    const std::string& targetId,
    bool isAdditive
) {
    const std::string anchorId = state.anchorId.value_or(
        state.selectedIds.empty() ? targetId : state.selectedIds.back()
    );

    const auto rangeIds = getRangeIds(orderedIds, anchorId, targetId);

    std::vector<std::string> selectedIds;
    if (isAdditive) {
        selectedIds = state.selectedIds;
        selectedIds.insert(selectedIds.end(), rangeIds.begin(), rangeIds.end());
        selectedIds = dedupeIds(selectedIds);
    } else {
        selectedIds = rangeIds;
    }

    return replaceSelection(selectedIds, anchorId);
}

SelectionState SelectionStateEngine::applyBoxSelection(const BoxSelectionChange& change) {
    std::vector<std::string> selectedIds;
    std::optional<std::string> anchorId;

    if (change.isAdditive) {
        selectedIds = change.initialSelectedIds;
        selectedIds.insert(selectedIds.end(), change.intersectedIds.begin(), change.intersectedIds.end());
        selectedIds = dedupeIds(selectedIds);

        anchorId = change.initialAnchorId.has_value()
            ? change.initialAnchorId
            : (change.intersectedIds.empty() ? std::nullopt : std::optional<std::string>(change.intersectedIds.back()));
    } else {
        selectedIds = change.intersectedIds;
        anchorId = change.intersectedIds.empty()
            ? std::nullopt
            : std::optional<std::string>(change.intersectedIds.back());
    }

    return replaceSelection(selectedIds, anchorId);
}

void SelectionStateEngine::activateScope(ScopeEntry entry) {
    if (s_activeScope.has_value() && s_activeScope->clear) {
        s_activeScope->clear();
    }
    s_activeScope = std::move(entry);
}

bool SelectionStateEngine::clearActiveScope() {
    if (!s_activeScope.has_value() || !s_activeScope->hasSelection || !s_activeScope->hasSelection()) {
        return false;
    }

    if (s_activeScope->clearActive) {
        s_activeScope->clearActive();
    } else if (s_activeScope->clear) {
        s_activeScope->clear();
    }
    return true;
}

bool SelectionStateEngine::hasActiveScopeSelection() {
    return s_activeScope.has_value() && s_activeScope->hasSelection && s_activeScope->hasSelection();
}

void SelectionStateEngine::resetActiveScope() noexcept {
    s_activeScope.reset();
}

} // namespace catchim::editor
