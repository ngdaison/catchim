#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace catchim::editor {

class PropertiesPanelStoreEngine {
public:
    PropertiesPanelStoreEngine() = default;

    const std::unordered_map<std::string, std::string>& activeTabPerType() const noexcept {
        return activeTabPerType_;
    }

    std::string getActiveTab(const std::string& elementType, const std::string& defaultTab = "basic") const;
    void setActiveTab(const std::string& elementType, const std::string& tabId);

    bool isTransformScaleLocked() const noexcept { return isTransformScaleLocked_; }
    void setTransformScaleLocked(bool locked) noexcept { isTransformScaleLocked_ = locked; }
    void toggleTransformScaleLock() noexcept { isTransformScaleLocked_ = !isTransformScaleLocked_; }

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);

private:
    std::unordered_map<std::string, std::string> activeTabPerType_;
    bool isTransformScaleLocked_{false};
};

} // namespace catchim::editor
