#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <optional>
#include <algorithm>

namespace catchim::editor {

struct SelectionState {
    std::vector<std::string> selectedIds;
    std::optional<std::string> anchorId;

    [[nodiscard]] bool empty() const noexcept { return selectedIds.empty(); }
    [[nodiscard]] size_t size() const noexcept { return selectedIds.size(); }
};

class SelectionManager {
public:
    SelectionManager() = default;

    [[nodiscard]] const SelectionState& getState() const noexcept { return state_; }
    [[nodiscard]] const std::vector<std::string>& getSelectedIds() const noexcept { return state_.selectedIds; }
    [[nodiscard]] const std::optional<std::string>& getAnchorId() const noexcept { return state_.anchorId; }
    [[nodiscard]] bool isSelected(const std::string& id) const noexcept;
    [[nodiscard]] bool empty() const noexcept { return state_.empty(); }
    [[nodiscard]] size_t count() const noexcept { return state_.size(); }

    void select(const std::string& id);
    void toggle(const std::string& id);
    void selectRange(
        const std::string& targetId,
        const std::vector<std::string>& orderedIds,
        bool isAdditive
    );
    void applyBoxSelection(
        const std::vector<std::string>& intersectedIds,
        bool isAdditive
    );
    void replaceSelection(
        const std::vector<std::string>& ids,
        std::optional<std::string> anchorId = std::nullopt
    );
    void clear() noexcept;
    void prune(const std::unordered_set<std::string>& validIds);

private:
    SelectionState state_;
    static std::vector<std::string> dedupeIds(const std::vector<std::string>& ids);
};

} // namespace catchim::editor
