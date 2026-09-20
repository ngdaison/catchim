#pragma once

#include "render/compositor/RenderSurface.h"
#include "render/effects/BlurEffect.h"
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <optional>

namespace catchim::render {

/**
 * @brief Generates standard 160x160 preview thumbnails for visual effects.
 * Corresponds to web/src/services/renderer/effect-preview.ts.
 */
class EffectPreviewService {
public:
    static constexpr int PREVIEW_SIZE = 160;

    EffectPreviewService();

    int previewSize() const noexcept { return PREVIEW_SIZE; }

    void renderPreview(
        const std::string& effectType,
        const nlohmann::json& params,
        RenderSurface& targetSurface,
        std::optional<std::pair<int, int>> uniformDimensions = std::nullopt
    );

    std::shared_ptr<RenderSurface> getTestSource(int width = PREVIEW_SIZE, int height = PREVIEW_SIZE);

private:
    std::shared_ptr<RenderSurface> testSource_;
};

} // namespace catchim::render
