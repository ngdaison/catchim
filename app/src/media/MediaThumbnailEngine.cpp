#include "media/MediaThumbnailEngine.h"
#include <cmath>
#include <algorithm>

namespace catchim::media {

ThumbnailDimensions MediaThumbnailEngine::calculateThumbnailSize(int width, int height) noexcept {
    if (width <= 0 || height <= 0) {
        return {0, 0};
    }

    const double aspectRatio = static_cast<double>(width) / static_cast<double>(height);
    int targetWidth = width;
    int targetHeight = height;

    if (targetWidth > THUMBNAIL_MAX_WIDTH) {
        targetWidth = THUMBNAIL_MAX_WIDTH;
        targetHeight = static_cast<int>(std::round(static_cast<double>(targetWidth) / aspectRatio));
    }
    if (targetHeight > THUMBNAIL_MAX_HEIGHT) {
        targetHeight = THUMBNAIL_MAX_HEIGHT;
        targetWidth = static_cast<int>(std::round(static_cast<double>(targetHeight) * aspectRatio));
    }

    return { targetWidth, targetHeight };
}

bool MediaThumbnailEngine::supportsAudio(MediaType type) noexcept {
    return type == MediaType::Video || type == MediaType::Audio;
}

bool MediaThumbnailEngine::supportsAudio(const MediaAsset* asset) noexcept {
    if (!asset) {
        return false;
    }
    return supportsAudio(asset->type());
}

std::optional<MediaType> MediaThumbnailEngine::detectMediaTypeFromMime(const std::string& mime) noexcept {
    if (mime.rfind("image/", 0) == 0) {
        return MediaType::Image;
    }
    if (mime.rfind("video/", 0) == 0) {
        return MediaType::Video;
    }
    if (mime.rfind("audio/", 0) == 0) {
        return MediaType::Audio;
    }
    return std::nullopt;
}

std::optional<MediaType> MediaThumbnailEngine::detectMediaTypeFromPath(const std::filesystem::path& path) noexcept {
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".webp" ||
        ext == ".gif" || ext == ".bmp" || ext == ".tiff" || ext == ".svg") {
        return MediaType::Image;
    }
    if (ext == ".mp4" || ext == ".mov" || ext == ".avi" || ext == ".mkv" ||
        ext == ".webm" || ext == ".m4v" || ext == ".flv" || ext == ".wmv") {
        return MediaType::Video;
    }
    if (ext == ".mp3" || ext == ".wav" || ext == ".aac" || ext == ".flac" ||
        ext == ".ogg" || ext == ".m4a" || ext == ".wma" || ext == ".opus") {
        return MediaType::Audio;
    }
    return std::nullopt;
}

std::optional<MediaType> MediaThumbnailEngine::detectMediaType(const std::string& mimeOrPath) noexcept {
    auto fromMime = detectMediaTypeFromMime(mimeOrPath);
    if (fromMime.has_value()) {
        return fromMime;
    }
    return detectMediaTypeFromPath(std::filesystem::path(mimeOrPath));
}

render::RenderSurface MediaThumbnailEngine::createThumbnailSurface(const render::RenderSurface& src) {
    if (src.width() <= 0 || src.height() <= 0) {
        return render::RenderSurface(0, 0);
    }

    auto targetDim = calculateThumbnailSize(src.width(), src.height());
    if (targetDim.width <= 0 || targetDim.height <= 0) {
        return render::RenderSurface(0, 0);
    }

    render::RenderSurface dst(targetDim.width, targetDim.height);

    const int srcW = src.width();
    const int srcH = src.height();
    const int dstW = targetDim.width;
    const int dstH = targetDim.height;

    // Fast-path: 1:1 identical dimensions
    if (srcW == dstW && srcH == dstH) {
        dst.copyFrom(src, 0, 0);
        return dst;
    }

    // High quality bilinear resampling
    const double scaleX = static_cast<double>(srcW) / static_cast<double>(dstW);
    const double scaleY = static_cast<double>(srcH) / static_cast<double>(dstH);

    for (int y = 0; y < dstH; ++y) {
        const double srcCenterY = (y + 0.5) * scaleY - 0.5;
        const int y0 = std::clamp(static_cast<int>(std::floor(srcCenterY)), 0, srcH - 1);
        const int y1 = std::clamp(y0 + 1, 0, srcH - 1);
        const double fy = std::clamp(srcCenterY - y0, 0.0, 1.0);

        for (int x = 0; x < dstW; ++x) {
            const double srcCenterX = (x + 0.5) * scaleX - 0.5;
            const int x0 = std::clamp(static_cast<int>(std::floor(srcCenterX)), 0, srcW - 1);
            const int x1 = std::clamp(x0 + 1, 0, srcW - 1);
            const double fx = std::clamp(srcCenterX - x0, 0.0, 1.0);

            uint8_t r00, g00, b00, a00;
            uint8_t r10, g10, b10, a10;
            uint8_t r01, g01, b01, a01;
            uint8_t r11, g11, b11, a11;

            render::RenderSurface::unpackRgba(src.getPixel(x0, y0), r00, g00, b00, a00);
            render::RenderSurface::unpackRgba(src.getPixel(x1, y0), r10, g10, b10, a10);
            render::RenderSurface::unpackRgba(src.getPixel(x0, y1), r01, g01, b01, a01);
            render::RenderSurface::unpackRgba(src.getPixel(x1, y1), r11, g11, b11, a11);

            const double w00 = (1.0 - fx) * (1.0 - fy);
            const double w10 = fx * (1.0 - fy);
            const double w01 = (1.0 - fx) * fy;
            const double w11 = fx * fy;

            const auto r = static_cast<uint8_t>(std::clamp(std::round(r00 * w00 + r10 * w10 + r01 * w01 + r11 * w11), 0.0, 255.0));
            const auto g = static_cast<uint8_t>(std::clamp(std::round(g00 * w00 + g10 * w10 + g01 * w01 + g11 * w11), 0.0, 255.0));
            const auto b = static_cast<uint8_t>(std::clamp(std::round(b00 * w00 + b10 * w10 + b01 * w01 + b11 * w11), 0.0, 255.0));
            const auto a = static_cast<uint8_t>(std::clamp(std::round(a00 * w00 + a10 * w10 + a01 * w01 + a11 * w11), 0.0, 255.0));

            dst.setPixel(x, y, render::RenderSurface::makeRgba(r, g, b, a));
        }
    }

    return dst;
}

} // namespace catchim::media
