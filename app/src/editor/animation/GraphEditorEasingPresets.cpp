#include "GraphEditorEasingPresets.h"
#include <cmath>
#include <algorithm>

namespace catchim::editor {

const std::vector<EasingPreset>& GraphEditorEasingPresets::builtinPresets() noexcept {
    static const std::vector<EasingPreset> kBuiltins = {
        {"smooth", "Smooth", core::CubicBezier{0.25, 0.1, 0.25, 1.0}, false},
        {"ease-out", "Ease out", core::CubicBezier{0.0, 0.0, 0.2, 1.0}, false},
        {"ease-in", "Ease in", core::CubicBezier{0.8, 0.0, 1.0, 1.0}, false},
        {"ease-in-out", "In out", core::CubicBezier{0.4, 0.0, 0.2, 1.0}, false},
        {"pop", "Pop", core::CubicBezier{0.175, 0.885, 0.32, 1.275}, false},
        {"linear", "Linear", core::CubicBezier{0.0, 0.0, 1.0, 1.0}, false}
    };
    return kBuiltins;
}

static bool curvesMatch(const core::CubicBezier& a, const core::CubicBezier& b, double tol) noexcept {
    return std::abs(a.x1 - b.x1) <= tol &&
           std::abs(a.y1 - b.y1) <= tol &&
           std::abs(a.x2 - b.x2) <= tol &&
           std::abs(a.y2 - b.y2) <= tol;
}

std::optional<EasingPreset> GraphEditorEasingPresets::findMatchingPreset(
    const core::CubicBezier& curve,
    double tolerance,
    std::span<const EasingPreset> additionalPresets
) noexcept {
    for (const auto& preset : additionalPresets) {
        if (curvesMatch(curve, preset.curve, tolerance)) {
            return preset;
        }
    }

    for (const auto& preset : builtinPresets()) {
        if (curvesMatch(curve, preset.curve, tolerance)) {
            return preset;
        }
    }

    return std::nullopt;
}

void GraphEditorEasingPresets::addCustomPreset(
    std::string id,
    std::string label,
    core::CubicBezier curve
) {
    auto it = std::find_if(customPresets_.begin(), customPresets_.end(), [&](const EasingPreset& p) {
        return p.id == id;
    });

    if (it != customPresets_.end()) {
        it->label = std::move(label);
        it->curve = curve;
    } else {
        customPresets_.push_back(EasingPreset{
            .id = std::move(id),
            .label = std::move(label),
            .curve = curve,
            .isCustom = true
        });
    }
}

bool GraphEditorEasingPresets::removeCustomPreset(const std::string& id) {
    auto it = std::remove_if(customPresets_.begin(), customPresets_.end(), [&](const EasingPreset& p) {
        return p.id == id;
    });

    if (it != customPresets_.end()) {
        customPresets_.erase(it, customPresets_.end());
        return true;
    }
    return false;
}

std::vector<EasingPreset> GraphEditorEasingPresets::allPresets() const {
    std::vector<EasingPreset> result;
    const auto& builtins = builtinPresets();
    result.reserve(builtins.size() + customPresets_.size());
    result.insert(result.end(), builtins.begin(), builtins.end());
    result.insert(result.end(), customPresets_.begin(), customPresets_.end());
    return result;
}

} // namespace catchim::editor
