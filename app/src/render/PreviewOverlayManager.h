#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace catchim::render {

enum class OverlayMountKind {
    Hud,
    Scene,
    Viewport
};

enum class OverlayHudAnchor {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

enum class OverlayPlane {
    UnderInteraction,
    OverInteraction
};

struct OverlayDefinition {
    std::string id;
    std::string label;
    bool defaultVisible{true};

    bool operator==(const OverlayDefinition& other) const = default;
};

struct OverlayControl {
    std::string id;
    std::string label;
    bool isVisible{true};

    bool operator==(const OverlayControl& other) const = default;
};

struct OverlayMount {
    OverlayMountKind kind{OverlayMountKind::Hud};
    OverlayHudAnchor anchor{OverlayHudAnchor::TopLeft};
    int order{0};
    double x{0.0};
    double y{0.0};
    double width{0.0};
    double height{0.0};

    bool operator==(const OverlayMount& other) const = default;
};

struct OverlayInstance {
    std::string id;
    OverlayMount mount;
    OverlayPlane plane{OverlayPlane::OverInteraction};
    int zIndex{0};

    bool operator==(const OverlayInstance& other) const = default;
};

struct OverlaySourceResult {
    std::vector<OverlayDefinition> definitions;
    std::vector<OverlayInstance> instances;
};

/**
 * @brief Manages preview overlay definitions, visibility state, and multi-source overlay merging.
 * Corresponds to web/src/preview/overlays.ts.
 */
class PreviewOverlayManager {
public:
    PreviewOverlayManager() = default;

    void registerDefinition(OverlayDefinition def);
    const std::vector<OverlayDefinition>& definitions() const noexcept { return definitions_; }

    bool isOverlayVisible(const std::string& id) const;
    void setOverlayVisible(const std::string& id, bool visible);
    void resetVisibility();

    OverlayControl createControl(const OverlayDefinition& def) const;
    std::vector<OverlayControl> getControls() const;

    static OverlaySourceResult mergeSources(
        const std::vector<OverlaySourceResult>& sources
    );

private:
    std::vector<OverlayDefinition> definitions_;
    std::unordered_map<std::string, bool> visibilityState_;
};

} // namespace catchim::render
