#pragma once

#include "core/math/Bezier.h"
#include <string>
#include <vector>
#include <optional>
#include <span>

namespace catchim::editor {

struct EasingPreset {
    std::string id;
    std::string label;
    core::CubicBezier curve;
    bool isCustom{false};

    bool operator==(const EasingPreset& other) const = default;
};

class GraphEditorEasingPresets {
public:
    static constexpr double PRESET_MATCH_TOLERANCE = 0.02;

    static const std::vector<EasingPreset>& builtinPresets() noexcept;

    static std::optional<EasingPreset> findMatchingPreset(
        const core::CubicBezier& curve,
        double tolerance = PRESET_MATCH_TOLERANCE,
        std::span<const EasingPreset> additionalPresets = {}
    ) noexcept;

    // Custom preset management
    void addCustomPreset(std::string id, std::string label, core::CubicBezier curve);
    bool removeCustomPreset(const std::string& id);
    const std::vector<EasingPreset>& customPresets() const noexcept { return customPresets_; }
    void clearCustomPresets() noexcept { customPresets_.clear(); }

    std::vector<EasingPreset> allPresets() const;

private:
    std::vector<EasingPreset> customPresets_;
};

} // namespace catchim::editor
