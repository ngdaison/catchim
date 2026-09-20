#include "RenderEngine.h"
#include "media/decoder/NativeVideoDecoder.h"
#include "media/MediaLibrary.h"
#include <algorithm>
#include <cmath>

#if defined(HAVE_QT6)
#include <QImage>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QRect>
#include <QPolygon>
#include <QPoint>
#endif

namespace catchim::render {

RenderEngine::RenderEngine(int32_t width, int32_t height)
    : compositor_(width, height)
{
}

void RenderEngine::setCanvasSize(int32_t width, int32_t height) {
    compositor_.setCanvasSize(width, height);
}

static void applyAdjustmentToPixels(std::vector<uint8_t>& pixels, double brightness, double contrast, double saturation, double temperature, double tint) {
    if (pixels.empty()) return;
    if (std::abs(brightness) < 0.01 && std::abs(contrast) < 0.01 && std::abs(saturation) < 0.01 && std::abs(temperature) < 0.01 && std::abs(tint) < 0.01) return;

    double bFactor = brightness * 1.28;
    double cFactor = (contrast > 0) ? (1.0 + contrast / 100.0 * 2.0) : (1.0 + contrast / 100.0);
    double sFactor = (saturation > 0) ? (1.0 + saturation / 100.0 * 2.0) : (1.0 + saturation / 100.0);
    double tempFactor = temperature * 0.5;
    double tintFactor = tint * 0.5;

    for (size_t i = 0; i < pixels.size(); i += 4) {
        if (pixels[i + 3] == 0) continue;

        double r = pixels[i + 0];
        double g = pixels[i + 1];
        double b = pixels[i + 2];

        // Brightness
        r += bFactor;
        g += bFactor;
        b += bFactor;

        // Contrast
        r = (r - 128.0) * cFactor + 128.0;
        g = (g - 128.0) * cFactor + 128.0;
        b = (b - 128.0) * cFactor + 128.0;

        // Temperature & Tint
        r += tempFactor;
        b -= tempFactor;
        g += tintFactor;

        // Saturation
        double gray = 0.299 * r + 0.587 * g + 0.114 * b;
        r = gray + (r - gray) * sFactor;
        g = gray + (g - gray) * sFactor;
        b = gray + (b - gray) * sFactor;

        pixels[i + 0] = static_cast<uint8_t>(std::clamp(r, 0.0, 255.0));
        pixels[i + 1] = static_cast<uint8_t>(std::clamp(g, 0.0, 255.0));
        pixels[i + 2] = static_cast<uint8_t>(std::clamp(b, 0.0, 255.0));
    }
}

static void applyEffectToPixels(std::vector<uint8_t>& pixels, int w, int h, const std::string& effectName, double intensity) {
    if (pixels.empty() || effectName.empty() || intensity <= 0.001) return;

    if (effectName.find("Monochrome") != std::string::npos || effectName.find("Đen trắng") != std::string::npos || effectName.find("grayscale") != std::string::npos) {
        for (size_t i = 0; i < pixels.size(); i += 4) {
            uint8_t r = pixels[i + 0];
            uint8_t g = pixels[i + 1];
            uint8_t b = pixels[i + 2];
            uint8_t gray = static_cast<uint8_t>((r * 77 + g * 150 + b * 29) >> 8);
            pixels[i + 0] = static_cast<uint8_t>(r + (gray - r) * intensity);
            pixels[i + 1] = static_cast<uint8_t>(g + (gray - g) * intensity);
            pixels[i + 2] = static_cast<uint8_t>(b + (gray - b) * intensity);
        }
    } else if (effectName.find("Invert") != std::string::npos || effectName.find("Đảo màu") != std::string::npos) {
        for (size_t i = 0; i < pixels.size(); i += 4) {
            uint8_t r = pixels[i + 0];
            uint8_t g = pixels[i + 1];
            uint8_t b = pixels[i + 2];
            pixels[i + 0] = static_cast<uint8_t>(r + ((255 - r) - r) * intensity);
            pixels[i + 1] = static_cast<uint8_t>(g + ((255 - g) - g) * intensity);
            pixels[i + 2] = static_cast<uint8_t>(b + ((255 - b) - b) * intensity);
        }
    } else if (effectName.find("Vignette") != std::string::npos || effectName.find("Tối viền") != std::string::npos) {
        double cx = w / 2.0;
        double cy = h / 2.0;
        double maxDist = std::sqrt(cx * cx + cy * cy);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                size_t i = (y * w + x) * 4;
                double dist = std::sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy)) / maxDist;
                double factor = std::clamp(1.0 - (dist * dist * intensity), 0.0, 1.0);
                pixels[i + 0] = static_cast<uint8_t>(pixels[i + 0] * factor);
                pixels[i + 1] = static_cast<uint8_t>(pixels[i + 1] * factor);
                pixels[i + 2] = static_cast<uint8_t>(pixels[i + 2] * factor);
            }
        }
    }
}

const CompositorOutput& RenderEngine::renderFrame(
    const editor::Project& project,
    const media::MediaLibrary& mediaLibrary,
    core::TimelineTime time
) {
    // 1. Clear background
    const auto& bg = project.settings().background;
    uint8_t r = 0, g = 0, b = 0;
    if (bg.type == editor::ProjectBackground::Type::Color && !bg.color.empty()) {
        if (bg.color[0] == '#' && bg.color.size() == 7) {
            try {
                r = static_cast<uint8_t>(std::stoi(bg.color.substr(1, 2), nullptr, 16));
                g = static_cast<uint8_t>(std::stoi(bg.color.substr(3, 2), nullptr, 16));
                b = static_cast<uint8_t>(std::stoi(bg.color.substr(5, 2), nullptr, 16));
            } catch (...) {}
        }
    }
    compositor_.clear(r, g, b, 255);

    const editor::Timeline* tl = project.activeTimeline();
    if (!tl) return compositor_.getOutput();

    int32_t zIndex = 0;

    auto renderClipLayer = [&](const editor::Clip& clip) {
        if (time < clip.startTime() || time >= clip.endTime()) return;

        RenderLayer layer;
        layer.id = clip.id().str();
        layer.zIndex = zIndex++;
        layer.sourceWidth = compositor_.width();
        layer.sourceHeight = compositor_.height();

        // Extract Transform
        layer.transform.positionX = clip.getParam<double>("transform.positionX", 0.0);
        layer.transform.positionY = clip.getParam<double>("transform.positionY", 0.0);
        layer.transform.scaleX = clip.getParam<double>("transform.scaleX", 1.0);
        layer.transform.scaleY = clip.getParam<double>("transform.scaleY", 1.0);
        layer.transform.rotate = clip.getParam<double>("transform.rotate", 0.0);
        layer.transform.opacity = clip.getParam<double>("opacity", 1.0);

        bool decoded = false;

        // Try decoding real video or image frame if mediaId is present
        if (!clip.mediaId().isEmpty()) {
            auto asset = mediaLibrary.findAsset(clip.mediaId());
            if (asset) {
                double speed = clip.getParam<double>("speed", 1.0);
                if (speed <= 0.01) speed = 1.0;

                double clipElapsed = (time - clip.startTime()).toSeconds() * speed + clip.trimStart().toSeconds();
                int fw = 0, fh = 0;
                std::vector<uint8_t> framePixels;

                if (media::NativeVideoDecoder::instance().getFrame(asset->filePath(), clipElapsed, fw, fh, framePixels)) {
                    layer.sourceWidth = fw;
                    layer.sourceHeight = fh;
                    layer.rgbaPixels = std::move(framePixels);
                    decoded = true;
                }
            }
        }

        if (clip.type() == editor::ClipType::Text) {
            layer.rgbaPixels.assign(layer.sourceWidth * layer.sourceHeight * 4, 0);
#if defined(HAVE_QT6)
            std::string textContent = clip.getParam<std::string>("text.content", clip.name());
            if (textContent.empty()) textContent = clip.name();
            double fontSize = clip.getParam<double>("text.fontSize", 44.0);
            std::string colorStr = clip.getParam<std::string>("text.color", "#FFFFFF");
            std::string style = clip.getParam<std::string>("text.style", "regular");

            QImage img(layer.rgbaPixels.data(), layer.sourceWidth, layer.sourceHeight, layer.sourceWidth * 4, QImage::Format_RGBA8888);
            {
                QPainter painter(&img);
                painter.setRenderHint(QPainter::Antialiasing);
                painter.setRenderHint(QPainter::TextAntialiasing);

                QFont font("Segoe UI", static_cast<int>(fontSize));
                if (style == "bold" || style == "cyber") {
                    font.setBold(true);
                }
                painter.setFont(font);

                QColor textColor(QString::fromStdString(colorStr));
                if (!textColor.isValid()) textColor = Qt::white;

                QRect rect(40, 0, layer.sourceWidth - 80, layer.sourceHeight);
                int flags = Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap;
                if (style == "lower_third") {
                    rect = QRect(60, layer.sourceHeight - 160, layer.sourceWidth - 120, 100);
                    painter.fillRect(rect, QColor(15, 15, 20, 180));
                    flags = Qt::AlignLeft | Qt::AlignVCenter;
                } else if (style == "regular") {
                    rect = QRect(60, layer.sourceHeight - 220, layer.sourceWidth - 120, 180);
                    flags = Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWordWrap;
                }

                // Drop shadow
                painter.setPen(QColor(0, 0, 0, 220));
                painter.drawText(rect.translated(2, 2), flags, QString::fromStdString(textContent));

                painter.setPen(textColor);
                painter.drawText(rect, flags, QString::fromStdString(textContent));
            }
#endif
            decoded = true;
        } else if (clip.type() == editor::ClipType::Graphic) {
            layer.rgbaPixels.assign(layer.sourceWidth * layer.sourceHeight * 4, 0);
#if defined(HAVE_QT6)
            std::string shape = clip.getParam<std::string>("graphic.shape", "rectangle");
            std::string colorStr = clip.getParam<std::string>("graphic.color", "#38bdf8");
            bool isAdjustment = clip.getParam<bool>("isAdjustmentLayer", false);

            if (!isAdjustment) {
                QImage img(layer.rgbaPixels.data(), layer.sourceWidth, layer.sourceHeight, layer.sourceWidth * 4, QImage::Format_RGBA8888);
                {
                    QPainter painter(&img);
                    painter.setRenderHint(QPainter::Antialiasing);

                    QColor fillColor(QString::fromStdString(colorStr));
                    if (!fillColor.isValid()) fillColor = QColor("#38bdf8");

                    painter.setBrush(fillColor);
                    painter.setPen(Qt::NoPen);

                    int cx = layer.sourceWidth / 2;
                    int cy = layer.sourceHeight / 2;
                    int size = std::min(layer.sourceWidth, layer.sourceHeight) / 4;

                    if (shape == "circle") {
                        painter.drawEllipse(QPoint(cx, cy), size / 2, size / 2);
                    } else if (shape == "star") {
                        QPolygon star;
                        for (int i = 0; i < 5; ++i) {
                            double a1 = i * 4.0 * 3.14159265358979323846 / 5.0 - 3.14159265358979323846 / 2.0;
                            star << QPoint(cx + static_cast<int>(std::cos(a1) * size / 2),
                                           cy + static_cast<int>(std::sin(a1) * size / 2));
                        }
                        painter.drawPolygon(star);
                    } else {
                        // Rectangle
                        painter.drawRoundedRect(QRect(cx - size, cy - size / 2, size * 2, size), 8, 8);
                    }
                }
            }
#endif
            decoded = true;
        }

        if (!decoded) {
            layer.rgbaPixels.assign(layer.sourceWidth * layer.sourceHeight * 4, 0);
        }

        // Apply visual adjustments
        double adjBright = clip.getParam<double>("adjustment.brightness", 0.0);
        double adjContrast = clip.getParam<double>("adjustment.contrast", 0.0);
        double adjSat = clip.getParam<double>("adjustment.saturation", 0.0);
        double adjTemp = clip.getParam<double>("adjustment.temperature", 0.0);
        double adjTint = clip.getParam<double>("adjustment.tint", 0.0);
        applyAdjustmentToPixels(layer.rgbaPixels, adjBright, adjContrast, adjSat, adjTemp, adjTint);

        // Apply real visual effects if configured
        std::string effName = clip.getParam<std::string>("effect.name", "");
        double effIntensity = clip.getParam<double>("effect.intensity", 1.0);
        if (!effName.empty()) {
            applyEffectToPixels(layer.rgbaPixels, layer.sourceWidth, layer.sourceHeight, effName, effIntensity);
        }

        compositor_.compositeLayer(layer);
    };

    // Main track
    const auto& mainTrack = tl->mainTrack();
    if (!mainTrack.isHidden()) {
        for (const auto& clip : mainTrack.clips()) {
            renderClipLayer(clip);
        }
    }

    // Overlay tracks
    for (const auto& track : tl->overlayTracks()) {
        if (track.isHidden()) continue;
        for (const auto& clip : track.clips()) {
            renderClipLayer(clip);
        }
    }

    return compositor_.getOutput();
}

} // namespace catchim::render
