#pragma once

#include <string>
#include <unordered_set>
#include <functional>
#include <vector>
#include <nlohmann/json.hpp>

namespace catchim::editor {

class TimelineUiStore {
public:
    using ChangeListener = std::function<void()>;

    TimelineUiStore();

    bool isSnappingEnabled() const noexcept { return snappingEnabled_; }
    void setSnappingEnabled(bool enabled);
    void toggleSnapping();

    bool isRippleEditingEnabled() const noexcept { return rippleEditingEnabled_; }
    void setRippleEditingEnabled(bool enabled);
    void toggleRippleEditing();

    const std::unordered_set<std::string>& expandedElementIds() const noexcept { return expandedElementIds_; }
    bool isElementExpanded(const std::string& elementId) const;
    void setElementExpanded(const std::string& elementId, bool expanded);
    void toggleElementExpanded(const std::string& elementId);
    void clearExpandedElements();

    void addChangeListener(ChangeListener listener);
    void clearChangeListeners();

    nlohmann::json toJson() const;
    bool fromJson(const nlohmann::json& j);

private:
    void notifyListeners();

    bool snappingEnabled_{true};
    bool rippleEditingEnabled_{false};
    std::unordered_set<std::string> expandedElementIds_;
    std::vector<ChangeListener> listeners_;
};

} // namespace catchim::editor
