#include "editor/timeline/TimelineUiStore.h"

namespace catchim::editor {

TimelineUiStore::TimelineUiStore() = default;

void TimelineUiStore::setSnappingEnabled(bool enabled) {
    if (snappingEnabled_ != enabled) {
        snappingEnabled_ = enabled;
        notifyListeners();
    }
}

void TimelineUiStore::toggleSnapping() {
    snappingEnabled_ = !snappingEnabled_;
    notifyListeners();
}

void TimelineUiStore::setRippleEditingEnabled(bool enabled) {
    if (rippleEditingEnabled_ != enabled) {
        rippleEditingEnabled_ = enabled;
        notifyListeners();
    }
}

void TimelineUiStore::toggleRippleEditing() {
    rippleEditingEnabled_ = !rippleEditingEnabled_;
    notifyListeners();
}

bool TimelineUiStore::isElementExpanded(const std::string& elementId) const {
    return expandedElementIds_.find(elementId) != expandedElementIds_.end();
}

void TimelineUiStore::setElementExpanded(const std::string& elementId, bool expanded) {
    if (expanded) {
        if (expandedElementIds_.insert(elementId).second) {
            notifyListeners();
        }
    } else {
        if (expandedElementIds_.erase(elementId) > 0) {
            notifyListeners();
        }
    }
}

void TimelineUiStore::toggleElementExpanded(const std::string& elementId) {
    auto it = expandedElementIds_.find(elementId);
    if (it != expandedElementIds_.end()) {
        expandedElementIds_.erase(it);
    } else {
        expandedElementIds_.insert(elementId);
    }
    notifyListeners();
}

void TimelineUiStore::clearExpandedElements() {
    if (!expandedElementIds_.empty()) {
        expandedElementIds_.clear();
        notifyListeners();
    }
}

void TimelineUiStore::addChangeListener(ChangeListener listener) {
    if (listener) {
        listeners_.push_back(std::move(listener));
    }
}

void TimelineUiStore::clearChangeListeners() {
    listeners_.clear();
}

void TimelineUiStore::notifyListeners() {
    for (const auto& listener : listeners_) {
        if (listener) {
            listener();
        }
    }
}

nlohmann::json TimelineUiStore::toJson() const {
    nlohmann::json j;
    j["name"] = "timeline-store";
    j["snappingEnabled"] = snappingEnabled_;
    j["rippleEditingEnabled"] = rippleEditingEnabled_;
    return j;
}

bool TimelineUiStore::fromJson(const nlohmann::json& j) {
    if (!j.is_object()) {
        return false;
    }
    if (j.contains("snappingEnabled") && j["snappingEnabled"].is_boolean()) {
        snappingEnabled_ = j["snappingEnabled"].get<bool>();
    }
    if (j.contains("rippleEditingEnabled") && j["rippleEditingEnabled"].is_boolean()) {
        rippleEditingEnabled_ = j["rippleEditingEnabled"].get<bool>();
    }
    notifyListeners();
    return true;
}

} // namespace catchim::editor
