#include "editor/properties/PropertiesViewModel.h"
#include <algorithm>

namespace catchim::editor {

namespace {

Clip* getMutableSelectedClip(EditorEngine& engine) {
    const auto& selected = engine.selectedClips();
    if (selected.size() == 1) {
        auto* tl = engine.activeTimeline();
        return tl ? tl->findClip(selected[0]) : nullptr;
    }
    return nullptr;
}

} // namespace

PropertiesViewModel::PropertiesViewModel(EditorEngine& engine)
    : engine_(engine)
{
}

PropertySelectionState PropertiesViewModel::getSelectionState() const {
    size_t count = getSelectedCount();
    if (count == 0) return PropertySelectionState::Empty;
    if (count == 1) return PropertySelectionState::Single;
    return PropertySelectionState::Multiple;
}

size_t PropertiesViewModel::getSelectedCount() const {
    return engine_.selectedClips().size();
}

const Clip* PropertiesViewModel::getSelectedClip() const {
    const auto& selected = engine_.selectedClips();
    if (selected.size() == 1) {
        const auto* tl = engine_.activeTimeline();
        return tl ? tl->findClip(selected[0]) : nullptr;
    }
    return nullptr;
}

std::vector<PropertyTabInfo> PropertiesViewModel::getAvailableTabs() const {
    const auto* clip = getSelectedClip();
    if (!clip) return {};

    switch (clip->type()) {
        case ClipType::Video:
            return {
                {"transform", "Biến đổi"},
                {"speed", "Tốc độ"},
                {"audio", "Âm thanh"},
                {"effects", "Hiệu ứng"},
                {"masks", "Mặt nạ"}
            };
        case ClipType::Audio:
            return {
                {"audio", "Âm thanh"},
                {"speed", "Tốc độ"}
            };
        case ClipType::Text:
            return {
                {"text", "Văn bản"},
                {"transform", "Biến đổi"},
                {"effects", "Hiệu ứng"}
            };
        case ClipType::Graphic:
            return {
                {"graphic", "Đồ họa"},
                {"transform", "Biến đổi"},
                {"effects", "Hiệu ứng"}
            };
        default:
            return {
                {"transform", "Biến đổi"},
                {"effects", "Hiệu ứng"}
            };
    }
}

std::string PropertiesViewModel::getActiveTab() const {
    const auto* clip = getSelectedClip();
    if (!clip) return "";

    std::string typeStr = clipTypeToString(clip->type());
    auto it = activeTabPerType_.find(typeStr);
    auto tabs = getAvailableTabs();

    if (it != activeTabPerType_.end()) {
        bool isValid = std::any_of(tabs.begin(), tabs.end(), [&](const auto& t) {
            return t.id == it->second;
        });
        if (isValid) return it->second;
    }

    return tabs.empty() ? "" : tabs[0].id;
}

void PropertiesViewModel::setActiveTab(const std::string& tabId) {
    const auto* clip = getSelectedClip();
    if (!clip) return;
    std::string typeStr = clipTypeToString(clip->type());
    activeTabPerType_[typeStr] = tabId;
}

double PropertiesViewModel::getPositionX() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("transform.positionX", 0.0) : 0.0;
}

void PropertiesViewModel::setPositionX(double x) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("transform.positionX", x);
    }
}

double PropertiesViewModel::getPositionY() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("transform.positionY", 0.0) : 0.0;
}

void PropertiesViewModel::setPositionY(double y) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("transform.positionY", y);
    }
}

double PropertiesViewModel::getScaleX() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("transform.scaleX", 1.0) : 1.0;
}

void PropertiesViewModel::setScaleX(double sx) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("transform.scaleX", sx);
    }
}

double PropertiesViewModel::getScaleY() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("transform.scaleY", 1.0) : 1.0;
}

void PropertiesViewModel::setScaleY(double sy) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("transform.scaleY", sy);
    }
}

double PropertiesViewModel::getRotation() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("transform.rotate", 0.0) : 0.0;
}

void PropertiesViewModel::setRotation(double degrees) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("transform.rotate", degrees);
    }
}

double PropertiesViewModel::getOpacity() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("opacity", 1.0) : 1.0;
}

void PropertiesViewModel::setOpacity(double opacity) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("opacity", std::clamp(opacity, 0.0, 1.0));
    }
}

std::string PropertiesViewModel::getBlendMode() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<std::string>("blendMode", "normal") : "normal";
}

void PropertiesViewModel::setBlendMode(const std::string& mode) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("blendMode", mode);
    }
}

double PropertiesViewModel::getSpeed() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("speed", 1.0) : 1.0;
}

void PropertiesViewModel::setSpeed(double speed) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("speed", std::clamp(speed, 0.01, 5.0));
    }
}

double PropertiesViewModel::getVolumeDb() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("volume", 0.0) : 0.0;
}

void PropertiesViewModel::setVolumeDb(double db) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("volume", std::clamp(db, -60.0, 12.0));
    }
}

bool PropertiesViewModel::isMuted() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->isMuted() : false;
}

void PropertiesViewModel::setMuted(bool muted) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setMuted(muted);
    }
}

std::string PropertiesViewModel::getTextContent() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<std::string>("text", "") : "";
}

void PropertiesViewModel::setTextContent(const std::string& text) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("text", text);
    }
}

double PropertiesViewModel::getFontSize() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<double>("fontSize", 48.0) : 48.0;
}

void PropertiesViewModel::setFontSize(double size) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("fontSize", std::max(1.0, size));
    }
}

std::string PropertiesViewModel::getShapeType() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<std::string>("shapeType", "rectangle") : "rectangle";
}

void PropertiesViewModel::setShapeType(const std::string& type) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("shapeType", type);
    }
}

uint32_t PropertiesViewModel::getFillColor() const {
    const auto* clip = getSelectedClip();
    return clip ? clip->getParam<uint32_t>("fillColor", 0xFFFFFFFF) : 0xFFFFFFFF;
}

void PropertiesViewModel::setFillColor(uint32_t argb) {
    if (auto* clip = getMutableSelectedClip(engine_)) {
        clip->setParam("fillColor", argb);
    }
}

} // namespace catchim::editor
