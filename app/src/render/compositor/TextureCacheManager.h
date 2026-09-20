#pragma once

#include "render/compositor/RenderSurface.h"
#include "render/scene/FrameDescriptorBuilder.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <vector>

namespace catchim::render {

struct TextureCacheEntry {
    TextureUploadKind kind{TextureUploadKind::Rendered};
    std::string id;
    std::string contentHash;
    int width{0};
    int height{0};
    std::shared_ptr<RenderSurface> surface;
};

/**
 * @brief Manages GPU/Compositor texture caching and upload synchronization.
 * Corresponds to web/src/services/renderer/compositor/wasm-compositor.ts.
 */
class TextureCacheManager {
public:
    TextureCacheManager() = default;

    void syncTextures(const std::vector<TextureUploadDescriptor>& textures);

    std::shared_ptr<RenderSurface> getTexture(const std::string& id) const;

    void registerExternalTexture(
        const std::string& id,
        std::shared_ptr<RenderSurface> surface
    );

    void rasterizeRenderedTexture(
        const std::string& id,
        const std::string& contentHash,
        int width,
        int height,
        const std::function<void(RenderSurface&)>& drawFn
    );

    size_t cachedCount() const noexcept { return cache_.size(); }
    uint64_t cacheHits() const noexcept { return cacheHits_; }
    uint64_t cacheMisses() const noexcept { return cacheMisses_; }
    uint64_t totalUploadedPixels() const noexcept { return totalUploadedPixels_; }

    void clear() noexcept;

private:
    std::unordered_map<std::string, TextureCacheEntry> cache_;
    uint64_t cacheHits_{0};
    uint64_t cacheMisses_{0};
    uint64_t totalUploadedPixels_{0};
};

} // namespace catchim::render
