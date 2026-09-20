#pragma once

#include "editor/EditorEngine.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace catchim::editor {

enum class PropertySelectionState {
    Empty,
    Single,
    Multiple
};

struct PropertyTabInfo {
    std::string id;
    std::string label;
};

class PropertiesViewModel {
public:
    explicit PropertiesViewModel(EditorEngine& engine);

    PropertySelectionState getSelectionState() const;
    size_t getSelectedCount() const;

    // Active Clip & Tabs (for Single selection)
    const Clip* getSelectedClip() const;
    std::vector<PropertyTabInfo> getAvailableTabs() const;

    std::string getActiveTab() const;
    void setActiveTab(const std::string& tabId);

    // Transform properties
    double getPositionX() const;
    void setPositionX(double x);

    double getPositionY() const;
    void setPositionY(double y);

    double getScaleX() const;
    void setScaleX(double sx);

    double getScaleY() const;
    void setScaleY(double sy);

    double getRotation() const;
    void setRotation(double degrees);

    double getOpacity() const;
    void setOpacity(double opacity);

    std::string getBlendMode() const;
    void setBlendMode(const std::string& mode);

    // Speed properties
    double getSpeed() const;
    void setSpeed(double speed);

    // Audio properties
    double getVolumeDb() const;
    void setVolumeDb(double db);

    bool isMuted() const;
    void setMuted(bool muted);

    // Text properties
    std::string getTextContent() const;
    void setTextContent(const std::string& text);

    double getFontSize() const;
    void setFontSize(double size);

    // Graphic / Shape properties
    std::string getShapeType() const;
    void setShapeType(const std::string& type);

    uint32_t getFillColor() const;
    void setFillColor(uint32_t argb);

private:
    EditorEngine& engine_;
    std::unordered_map<std::string, std::string> activeTabPerType_;
};

} // namespace catchim::editor
