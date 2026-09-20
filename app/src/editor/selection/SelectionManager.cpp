#include "editor/selection/SelectionManager.h"
#include <unordered_set>

namespace catchim::editor {

std::vector<std::string> SelectionManager::dedupeIds(const std::vector<std::string>& ids) {
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

bool SelectionManager::isSelected(const std::string& id) const noexcept {
    return std::find(state_.selectedIds.begin(), state_.selectedIds.end(), id) != state_.selectedIds.end();
}

void SelectionManager::select(const std::string& id) {
    state_.selectedIds = {id};
    state_.anchorId = id;
}

void SelectionManager::toggle(const std::string& id) {
    auto it = std::find(state_.selectedIds.begin(), state_.selectedIds.end(), id);
    if (it != state_.selectedIds.end()) {
        state_.selectedIds.erase(it);
        if (state_.anchorId == id) {
            state_.anchorId = state_.selectedIds.empty() ? std::nullopt : std::optional<std::string>(state_.selectedIds.back());
        }
    } else {
        state_.selectedIds.push_back(id);
        state_.anchorId = id;
    }
}

void SelectionManager::selectRange(
    const std::string& targetId,
    const std::vector<std::string>& orderedIds,
    bool isAdditive
) {
    std::string anchor = state_.anchorId.value_or(
        state_.selectedIds.empty() ? targetId : state_.selectedIds.back()
    );

    auto anchorIt = std::find(orderedIds.begin(), orderedIds.end(), anchor);
    auto targetIt = std::find(orderedIds.begin(), orderedIds.end(), targetId);

    if (anchorIt == orderedIds.end() || targetIt == orderedIds.end()) {
        replaceSelection({targetId}, targetId);
        return;
    }

    size_t anchorIdx = std::distance(orderedIds.begin(), anchorIt);
    size_t targetIdx = std::distance(orderedIds.begin(), targetIt);

    size_t startIdx = std::min(anchorIdx, targetIdx);
    size_t endIdx = std::max(anchorIdx, targetIdx);

    std::vector<std::string> rangeIds;
    rangeIds.reserve(endIdx - startIdx + 1);
    for (size_t i = startIdx; i <= endIdx; ++i) {
        rangeIds.push_back(orderedIds[i]);
    }

    if (isAdditive) {
        std::vector<std::string> combined = state_.selectedIds;
        combined.insert(combined.end(), rangeIds.begin(), rangeIds.end());
        replaceSelection(combined, anchor);
    } else {
        replaceSelection(rangeIds, anchor);
    }
}

void SelectionManager::applyBoxSelection(
    const std::vector<std::string>& intersectedIds,
    bool isAdditive
) {
    if (isAdditive) {
        std::vector<std::string> combined = state_.selectedIds;
        combined.insert(combined.end(), intersectedIds.begin(), intersectedIds.end());
        std::optional<std::string> anchor = state_.anchorId;
        if (!anchor.has_value() && !intersectedIds.empty()) {
            anchor = intersectedIds.back();
        }
        replaceSelection(combined, anchor);
    } else {
        std::optional<std::string> anchor = intersectedIds.empty() ? std::nullopt : std::optional<std::string>(intersectedIds.back());
        replaceSelection(intersectedIds, anchor);
    }
}

void SelectionManager::replaceSelection(
    const std::vector<std::string>& ids,
    std::optional<std::string> anchorId
) {
    state_.selectedIds = dedupeIds(ids);
    if (anchorId.has_value()) {
        state_.anchorId = anchorId;
    } else if (!state_.selectedIds.empty()) {
        state_.anchorId = state_.selectedIds.back();
    } else {
        state_.anchorId = std::nullopt;
    }
}

void SelectionManager::clear() noexcept {
    state_.selectedIds.clear();
    state_.anchorId = std::nullopt;
}

void SelectionManager::prune(const std::unordered_set<std::string>& validIds) {
    std::vector<std::string> pruned;
    pruned.reserve(state_.selectedIds.size());
    for (const auto& id : state_.selectedIds) {
        if (validIds.find(id) != validIds.end()) {
            pruned.push_back(id);
        }
    }
    state_.selectedIds = std::move(pruned);
    if (state_.anchorId.has_value() && validIds.find(*state_.anchorId) == validIds.end()) {
        state_.anchorId = state_.selectedIds.empty() ? std::nullopt : std::optional<std::string>(state_.selectedIds.back());
    }
}

} // namespace catchim::editor
