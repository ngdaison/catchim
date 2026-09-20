#include "render/masks/MaskRegistry.h"
#include "render/masks/BuiltinMaskGeometry.h"
#include <algorithm>

namespace catchim::render {

static const std::vector<std::string> kAllMaskTypes = {
    "split",
    "cinematic-bars",
    "rectangle",
    "ellipse",
    "heart",
    "diamond",
    "star",
    "text",
    "freeform"
};

const std::vector<std::string>& MaskRegistry::getAllMaskTypes() {
    return kAllMaskTypes;
}

bool MaskRegistry::isRegistered(const std::string& type) noexcept {
    return std::find(kAllMaskTypes.begin(), kAllMaskTypes.end(), type) != kAllMaskTypes.end();
}

MaskFeatures MaskRegistry::getFeatures(const std::string& type) noexcept {
    if (type == "split" || type == "freeform") {
        return MaskFeatures{ .hasPosition = true, .hasRotation = true, .sizeMode = "none" };
    }
    if (type == "cinematic-bars") {
        return MaskFeatures{ .hasPosition = true, .hasRotation = true, .sizeMode = "height-only" };
    }
    if (type == "rectangle" || type == "ellipse" || type == "heart" ||
        type == "diamond" || type == "star") {
        return MaskFeatures{ .hasPosition = true, .hasRotation = true, .sizeMode = "width-height" };
    }
    if (type == "text") {
        return MaskFeatures{ .hasPosition = true, .hasRotation = true, .sizeMode = "uniform" };
    }
    return MaskFeatures{ .hasPosition = true, .hasRotation = true, .sizeMode = "none" };
}

std::string MaskRegistry::getName(const std::string& type) {
    if (type == "split") return "Split";
    if (type == "cinematic-bars") return "Cinematic Bars";
    if (type == "rectangle") return "Rectangle";
    if (type == "ellipse") return "Ellipse";
    if (type == "heart") return "Heart";
    if (type == "diamond") return "Diamond";
    if (type == "star") return "Star";
    if (type == "text") return "Text";
    if (type == "freeform") return "Freeform";
    return "Custom";
}

nlohmann::json MaskRegistry::buildDefault(
    const std::string& type,
    double elementWidth,
    double elementHeight
) {
    nlohmann::json base = {
        {"feather", 0.0},
        {"inverted", false},
        {"strokeColor", "#ffffff"},
        {"strokeWidth", 0.0},
        {"strokeAlign", "center"}
    };

    if (type == "split") {
        base["centerX"] = 0.0;
        base["centerY"] = 0.0;
        base["rotation"] = 0.0;
        return base;
    }

    if (type == "cinematic-bars") {
        auto p = BuiltinMaskGeometry::getDefaultCinematicBarsMaskParams(elementWidth, elementHeight);
        base["centerX"] = p.centerX;
        base["centerY"] = p.centerY;
        base["width"] = p.width;
        base["height"] = p.height;
        base["rotation"] = p.rotation;
        base["scale"] = p.scale;
        return base;
    }

    if (type == "rectangle" || type == "ellipse" || type == "heart" ||
        type == "diamond" || type == "star") {
        auto p = BuiltinMaskGeometry::getDefaultSquareMaskParams(elementWidth, elementHeight);
        base["centerX"] = p.centerX;
        base["centerY"] = p.centerY;
        base["width"] = p.width;
        base["height"] = p.height;
        base["rotation"] = p.rotation;
        base["scale"] = p.scale;
        return base;
    }

    if (type == "text") {
        base["content"] = "Mask";
        base["fontSize"] = 15;
        base["fontFamily"] = "Arial";
        base["fontWeight"] = "normal";
        base["fontStyle"] = "normal";
        base["textDecoration"] = "none";
        base["letterSpacing"] = 0.0;
        base["lineHeight"] = 1.2;
        base["centerX"] = 0.0;
        base["centerY"] = 0.0;
        base["rotation"] = 0.0;
        base["scale"] = 1.0;
        return base;
    }

    if (type == "freeform") {
        base["path"] = nlohmann::json::array();
        base["closed"] = true;
        base["centerX"] = 0.0;
        base["centerY"] = 0.0;
        base["rotation"] = 0.0;
        base["scale"] = 1.0;
        return base;
    }

    return base;
}

bool MaskRegistry::isActive(const std::string& type, const nlohmann::json& params) {
    if (type == "text") {
        if (!params.contains("content")) {
            return false;
        }
        std::string content = params["content"].is_string() ? params["content"].get<std::string>() : "";
        // trim whitespace
        size_t first = content.find_first_not_of(" \t\n\r");
        return (first != std::string::npos);
    }
    return true;
}

} // namespace catchim::render
