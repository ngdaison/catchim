#include "export/StillImageExporter.h"
#include <fstream>
#include <cstring>

namespace catchim::exporting {

std::vector<uint8_t> StillImageExporter::renderFrameRgba(
    const editor::Project& project,
    core::TimelineTime time,
    int32_t width,
    int32_t height
) {
    render::Compositor compositor(width, height);
    compositor.clear(0, 0, 0, 255); // Default black background

    const auto* scene = project.activeScene();
    if (!scene) return compositor.getOutput().rgbaPixels;

    // Composite all active clips at time
    for (const auto* track : scene->timeline().allTracks()) {
        if (track->isHidden()) continue;
        for (const auto& clip : track->clips()) {
            if (clip.isHidden()) continue;
            if (time >= clip.startTime() && time < clip.endTime()) {
                render::RenderLayer layer;
                layer.id = clip.id().str();
                layer.sourceWidth = width;
                layer.sourceHeight = height;
                layer.rgbaPixels.resize(static_cast<size_t>(width * height * 4), 255);
                compositor.compositeLayer(layer);
            }
        }
    }

    return compositor.getOutput().rgbaPixels;
}

std::vector<uint8_t> StillImageExporter::encodeBmp(
    const uint8_t* rgba,
    int32_t w,
    int32_t h
) {
    if (!rgba || w <= 0 || h <= 0) return {};

    uint32_t imageSize = static_cast<uint32_t>(w * h * 4);
    uint32_t fileSize = 14 + 40 + imageSize;

    std::vector<uint8_t> bmp(fileSize, 0);

    // 1. BMP Header (14 bytes)
    bmp[0] = 'B';
    bmp[1] = 'M';
    std::memcpy(&bmp[2], &fileSize, 4);
    uint32_t dataOffset = 54;
    std::memcpy(&bmp[10], &dataOffset, 4);

    // 2. DIB Header (40 bytes)
    uint32_t dibHeaderSize = 40;
    std::memcpy(&bmp[14], &dibHeaderSize, 4);
    std::memcpy(&bmp[18], &w, 4);
    int32_t negH = -h; // Top-down scanline order
    std::memcpy(&bmp[22], &negH, 4);
    uint16_t planes = 1;
    std::memcpy(&bmp[26], &planes, 2);
    uint16_t bpp = 32;
    std::memcpy(&bmp[28], &bpp, 2);
    std::memcpy(&bmp[34], &imageSize, 4);

    // 3. Pixel data in BGRA order
    uint8_t* dst = &bmp[54];
    for (int32_t i = 0; i < w * h; ++i) {
        dst[i * 4 + 0] = rgba[i * 4 + 2]; // B
        dst[i * 4 + 1] = rgba[i * 4 + 1]; // G
        dst[i * 4 + 2] = rgba[i * 4 + 0]; // R
        dst[i * 4 + 3] = rgba[i * 4 + 3]; // A
    }

    return bmp;
}

bool StillImageExporter::saveSnapshot(
    const editor::Project& project,
    core::TimelineTime time,
    const std::string& outputPath,
    int32_t w,
    int32_t h
) {
    auto rgba = renderFrameRgba(project, time, w, h);
    auto bmp = encodeBmp(rgba.data(), w, h);
    if (bmp.empty()) return false;

    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs.is_open()) return false;
    ofs.write(reinterpret_cast<const char*>(bmp.data()), bmp.size());
    return true;
}

} // namespace catchim::exporting
