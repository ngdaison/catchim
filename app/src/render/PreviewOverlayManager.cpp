#include "render/PreviewOverlayManager.h"
#include <algorithm>
#include <unordered_set>

namespace catchim::render {

void PreviewOverlayManager::registerDefinition(OverlayDefinition def) {
    auto it = std::find_if(definitions_.begin(), definitions_.end(), [&](const OverlayDefinition& item) {
        return item.id == def.id;
    });

    if (it != definitions_.end()) {
        *it = std::move(def);
    } else {
        definitions_.push_back(std::move(def));
    }
}

bool PreviewOverlayManager::isOverlayVisible(const std::string& id) const {
    auto it = visibilityState_.find(id);
    if (it != visibilityState_.end()) {
        return it->second;
    }

    auto defIt = std::find_if(definitions_.begin(), definitions_.end(), [&](const OverlayDefinition& item) {
        return item.id == id;
    });

    if (defIt != definitions_.end()) {
        return defIt->defaultVisible;
    }

    return true;
}

void PreviewOverlayManager::setOverlayVisible(const std::string& id, bool visible) {
    visibilityState_[id] = visible;
}

void PreviewOverlayManager::resetVisibility() {
    visibilityState_.clear();
}

OverlayControl PreviewOverlayManager::createControl(const OverlayDefinition& def) const {
    return OverlayControl{
        .id = def.id,
        .label = def.label,
        .isVisible = isOverlayVisible(def.id)
    };
}

std::vector<OverlayControl> PreviewOverlayManager::getControls() const {
    std::vector<OverlayControl> controls;
    controls.reserve(definitions_.size());
    for (const auto& def : definitions_) {
        controls.push_back(createControl(def));
    }
    return controls;
}

OverlaySourceResult PreviewOverlayManager::mergeSources(
    const std::vector<OverlaySourceResult>& sources
) {
    OverlaySourceResult result;
    std::unordered_set<std::string> seenIds;

    for (const auto& source : sources) {
        for (const auto& def : source.definitions) {
            if (!seenIds.contains(def.id)) {
                seenIds.insert(def.id);
                result.definitions.push_back(def);
            }
        }
        for (const auto& inst : source.instances) {
            result.instances.push_back(inst);
        }
    }

    return result;
}

} // namespace catchim::render
