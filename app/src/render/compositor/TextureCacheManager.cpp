#include "render/compositor/TextureCacheManager.h"
#include "render/diagnostics/RenderPerformanceProfiler.h"
#include <unordered_set>

namespace catchim::render {

void TextureCacheManager::syncTextures(const std::vector<TextureUploadDescriptor>& textures) {
    std::unordered_set<std::string> nextIds;
    nextIds.reserve(textures.size());
    for (const auto& t : textures) {
        nextIds.insert(t.id);
    }

    // Evict textures that are no longer referenced in this frame
    for (auto it = cache_.begin(); it != cache_.end(); ) {
        if (!nextIds.contains(it->first)) {
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }

    auto& profiler = RenderPerformanceProfiler::instance();

    for (const auto& tex : textures) {
        auto it = cache_.find(tex.id);
        if (tex.kind == TextureUploadKind::External) {
            if (it != cache_.end() && it->second.kind == TextureUploadKind::External &&
                it->second.width == tex.width && it->second.height == tex.height) {
                ++cacheHits_;
                profiler.incrementCounter("textureCacheHit", 1);
                continue;
            }

            ++cacheMisses_;
            uint64_t pixels = static_cast<uint64_t>(tex.width) * static_cast<uint64_t>(tex.height);
            totalUploadedPixels_ += pixels;
            profiler.incrementCounter("textureUpload", 1);
            profiler.incrementCounter("textureUploadPixels", static_cast<int64_t>(pixels));

            auto surface = std::make_shared<RenderSurface>(tex.width, tex.height);
            cache_[tex.id] = TextureCacheEntry{
                .kind = TextureUploadKind::External,
                .id = tex.id,
                .contentHash = tex.contentHash,
                .width = tex.width,
                .height = tex.height,
                .surface = std::move(surface)
            };
        } else { // Rendered
            if (it != cache_.end() && it->second.kind == TextureUploadKind::Rendered &&
                it->second.contentHash == tex.contentHash &&
                it->second.width == tex.width && it->second.height == tex.height) {
                ++cacheHits_;
                profiler.incrementCounter("textureCacheHit", 1);
                continue;
            }

            ++cacheMisses_;
            uint64_t pixels = static_cast<uint64_t>(tex.width) * static_cast<uint64_t>(tex.height);
            totalUploadedPixels_ += pixels;
            profiler.incrementCounter("textureUpload", 1);
            profiler.incrementCounter("textureUploadPixels", static_cast<int64_t>(pixels));

            std::shared_ptr<RenderSurface> surface;
            if (it != cache_.end() && it->second.surface &&
                it->second.width == tex.width && it->second.height == tex.height) {
                surface = it->second.surface;
            } else {
                surface = std::make_shared<RenderSurface>(tex.width, tex.height);
            }

            cache_[tex.id] = TextureCacheEntry{
                .kind = TextureUploadKind::Rendered,
                .id = tex.id,
                .contentHash = tex.contentHash,
                .width = tex.width,
                .height = tex.height,
                .surface = std::move(surface)
            };
        }
    }
}

std::shared_ptr<RenderSurface> TextureCacheManager::getTexture(const std::string& id) const {
    auto it = cache_.find(id);
    if (it != cache_.end()) {
        return it->second.surface;
    }
    return nullptr;
}

void TextureCacheManager::registerExternalTexture(
    const std::string& id,
    std::shared_ptr<RenderSurface> surface
) {
    if (!surface) return;
    cache_[id] = TextureCacheEntry{
        .kind = TextureUploadKind::External,
        .id = id,
        .contentHash = "",
        .width = surface->width(),
        .height = surface->height(),
        .surface = std::move(surface)
    };
}

void TextureCacheManager::rasterizeRenderedTexture(
    const std::string& id,
    const std::string& contentHash,
    int width,
    int height,
    const std::function<void(RenderSurface&)>& drawFn
) {
    auto it = cache_.find(id);
    if (it != cache_.end() && it->second.kind == TextureUploadKind::Rendered &&
        it->second.contentHash == contentHash &&
        it->second.width == width && it->second.height == height) {
        ++cacheHits_;
        RenderPerformanceProfiler::instance().incrementCounter("textureCacheHit", 1);
        return;
    }

    ++cacheMisses_;
    uint64_t pixels = static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
    totalUploadedPixels_ += pixels;
    RenderPerformanceProfiler::instance().incrementCounter("textureUpload", 1);
    RenderPerformanceProfiler::instance().incrementCounter("textureUploadPixels", static_cast<int64_t>(pixels));

    std::shared_ptr<RenderSurface> surface;
    if (it != cache_.end() && it->second.surface &&
        it->second.width == width && it->second.height == height) {
        surface = it->second.surface;
    } else {
        surface = std::make_shared<RenderSurface>(width, height);
    }

    surface->clear();
    if (drawFn) {
        drawFn(*surface);
    }

    cache_[id] = TextureCacheEntry{
        .kind = TextureUploadKind::Rendered,
        .id = id,
        .contentHash = contentHash,
        .width = width,
        .height = height,
        .surface = std::move(surface)
    };
}

void TextureCacheManager::clear() noexcept {
    cache_.clear();
    cacheHits_ = 0;
    cacheMisses_ = 0;
    totalUploadedPixels_ = 0;
}

} // namespace catchim::render
