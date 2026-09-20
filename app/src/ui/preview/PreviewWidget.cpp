#include "PreviewWidget.h"

#if defined(HAVE_QT6)
#include <QPainter>
#include <QMouseEvent>
#include <QColor>
#include <cmath>
#include <algorithm>

namespace catchim::ui {

PreviewWidget::PreviewWidget(
    editor::EditorEngine& engine,
    render::RenderEngine& renderEngine,
    media::MediaLibrary& mediaLibrary,
    QWidget* parent
)
    : QWidget(parent)
    , engine_(engine)
    , renderEngine_(renderEngine)
    , mediaLibrary_(mediaLibrary)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

PreviewWidget::HandleType PreviewWidget::hitTestHandles(const QPoint& mousePos, const QRect& clipRect) const {
    const int handleRadius = 6;

    // Check Rotate handle (20px above top center)
    QPoint rotCenter(clipRect.center().x(), clipRect.top() - 20);
    if ((mousePos - rotCenter).manhattanLength() <= handleRadius + 2) {
        return HandleType::Rotate;
    }

    auto nearPoint = [handleRadius](const QPoint& p1, const QPoint& p2) {
        return std::abs(p1.x() - p2.x()) <= handleRadius && std::abs(p1.y() - p2.y()) <= handleRadius;
    };

    if (nearPoint(mousePos, clipRect.topLeft())) return HandleType::TopLeft;
    if (nearPoint(mousePos, QPoint(clipRect.center().x(), clipRect.top()))) return HandleType::TopMid;
    if (nearPoint(mousePos, clipRect.topRight())) return HandleType::TopRight;
    if (nearPoint(mousePos, QPoint(clipRect.left(), clipRect.center().y()))) return HandleType::MidLeft;
    if (nearPoint(mousePos, QPoint(clipRect.right(), clipRect.center().y()))) return HandleType::MidRight;
    if (nearPoint(mousePos, clipRect.bottomLeft())) return HandleType::BotLeft;
    if (nearPoint(mousePos, QPoint(clipRect.center().x(), clipRect.bottom()))) return HandleType::BotMid;
    if (nearPoint(mousePos, clipRect.bottomRight())) return HandleType::BotRight;

    if (clipRect.contains(mousePos)) return HandleType::Move;

    return HandleType::None;
}

void PreviewWidget::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // Viewport background
    painter.fillRect(rect(), QColor("#09090b"));

    int canvasW = engine_.project().settings().canvasSize.width;
    int canvasH = engine_.project().settings().canvasSize.height;
    if (canvasW <= 0) canvasW = 1920;
    if (canvasH <= 0) canvasH = 1080;

    // Compute scale and bounds
    double scale = 1.0;
    if (zoomFactor_ < 0.0) {
        double scaleW = static_cast<double>(width() - 48) / canvasW;
        double scaleH = static_cast<double>(height() - 48) / canvasH;
        scale = std::max(0.05, std::min(scaleW, scaleH));
    } else {
        scale = zoomFactor_;
    }

    int dispW = static_cast<int>(canvasW * scale);
    int dispH = static_cast<int>(canvasH * scale);
    int startX = (width() - dispW) / 2 + panOffset_.x();
    int startY = (height() - dispH) / 2 + panOffset_.y();
    QRect canvasRect(startX, startY, dispW, dispH);

    // 1. Draw Canvas background color
    std::string customBg = engine_.project().settings().background.color;
    if (customBg.empty()) customBg = "#000000";
    painter.fillRect(canvasRect, QColor(QString::fromStdString(customBg)));

    // 2. Render core frame
    core::TimelineTime curTime = engine_.playback().currentTime();
    const auto& frame = renderEngine_.renderFrame(engine_.project(), mediaLibrary_, curTime);

    if (frame.width > 0 && frame.height > 0 && !frame.rgbaPixels.empty()) {
        QImage img(
            frame.rgbaPixels.data(),
            frame.width,
            frame.height,
            frame.width * 4,
            QImage::Format_RGBA8888
        );
        painter.drawImage(canvasRect, img);
    }

    // 3. Render active Text and Graphic elements if not already composited
    auto* tl = engine_.activeTimeline();
    if (tl) {
        for (const auto* track : tl->allTracks()) {
            if (track->isHidden()) continue;
            for (const auto& clip : track->clips()) {
                if (curTime < clip.startTime() || curTime >= clip.endTime()) continue;

                double px = clip.getParam<double>("transform.positionX", 0.0);
                double py = clip.getParam<double>("transform.positionY", 0.0);
                double sx = clip.getParam<double>("transform.scaleX", 1.0);
                double sy = clip.getParam<double>("transform.scaleY", 1.0);
                double opacity = clip.getParam<double>("opacity", 1.0);

                if (clip.type() == editor::ClipType::Text) {
                    std::string textContent = clip.getParam<std::string>("text.content", clip.name());
                    double fontSize = clip.getParam<double>("text.fontSize", 48.0) * scale * sx;
                    std::string textColor = clip.getParam<std::string>("text.color", "#FFFFFF");

                    painter.save();
                    painter.setOpacity(opacity);
                    QFont font("Inter", std::max(8, static_cast<int>(fontSize)), QFont::Bold);
                    painter.setFont(font);
                    painter.setPen(QColor(QString::fromStdString(textColor)));

                    int textCenterX = startX + dispW / 2 + static_cast<int>(px * scale);
                    int textCenterY = startY + dispH / 2 + static_cast<int>(py * scale);
                    QRect textBounds(textCenterX - dispW / 2, textCenterY - 40, dispW, 80);
                    painter.drawText(textBounds, Qt::AlignCenter, QString::fromStdString(textContent));
                    painter.restore();
                } else if (clip.type() == editor::ClipType::Graphic) {
                    std::string shape = clip.getParam<std::string>("graphic.shape", "rectangle");
                    std::string color = clip.getParam<std::string>("graphic.color", "#38bdf8");

                    painter.save();
                    painter.setOpacity(opacity);
                    painter.setBrush(QColor(QString::fromStdString(color)));
                    painter.setPen(Qt::NoPen);

                    int shapeW = static_cast<int>(200.0 * scale * sx);
                    int shapeH = static_cast<int>(200.0 * scale * sy);
                    int shapeX = startX + dispW / 2 + static_cast<int>(px * scale) - shapeW / 2;
                    int shapeY = startY + dispH / 2 + static_cast<int>(py * scale) - shapeH / 2;

                    if (shape == "circle") {
                        painter.drawEllipse(shapeX, shapeY, shapeW, shapeH);
                    } else {
                        painter.drawRoundedRect(shapeX, shapeY, shapeW, shapeH, 8, 8);
                    }
                    painter.restore();
                }
            }
        }
    }

    // 4. Draw Canvas outer border
    painter.setPen(QPen(QColor("#27272a"), 1.0));
    painter.drawRect(canvasRect);

    // 5. Draw Center Alignment Snapping Guides
    if (showCenterGuideX_) {
        painter.setPen(QPen(QColor("#ef4444"), 1.0, Qt::DashLine));
        painter.drawLine(startX + dispW / 2, startY, startX + dispW / 2, startY + dispH);
    }
    if (showCenterGuideY_) {
        painter.setPen(QPen(QColor("#ef4444"), 1.0, Qt::DashLine));
        painter.drawLine(startX, startY + dispH / 2, startX + dispW, startY + dispH / 2);
    }

    // 6. Draw Selected Clip Bounding Box & Transform Handles
    const auto& sel = engine_.selectedClips();
    if (!sel.empty() && tl) {
        const auto* clip = tl->findClip(sel[0]);
        if (clip) {
            double px = clip->getParam<double>("transform.positionX", 0.0);
            double py = clip->getParam<double>("transform.positionY", 0.0);
            double sx = clip->getParam<double>("transform.scaleX", 1.0);
            double sy = clip->getParam<double>("transform.scaleY", 1.0);

            int boxW = static_cast<int>(dispW * std::max(0.2, sx * 0.8));
            int boxH = static_cast<int>(dispH * std::max(0.2, sy * 0.8));
            if (clip->type() == editor::ClipType::Text || clip->type() == editor::ClipType::Graphic) {
                boxW = static_cast<int>(240 * scale * sx);
                boxH = static_cast<int>(100 * scale * sy);
            }

            int boxX = startX + dispW / 2 + static_cast<int>(px * scale) - boxW / 2;
            int boxY = startY + dispH / 2 + static_cast<int>(py * scale) - boxH / 2;
            QRect selRect(boxX, boxY, boxW, boxH);

            // Bounding box rectangle
            painter.setPen(QPen(QColor("#38bdf8"), 1.5));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(selRect);

            // Rotation stick and circle
            QPoint rotTop(selRect.center().x(), selRect.top() - 20);
            painter.drawLine(selRect.center().x(), selRect.top(), rotTop.x(), rotTop.y());
            painter.setBrush(QColor("#38bdf8"));
            painter.drawEllipse(rotTop, 5, 5);

            // 8 Handles
            auto drawHandle = [&painter](const QPoint& pt) {
                painter.fillRect(pt.x() - 4, pt.y() - 4, 8, 8, QColor("#ffffff"));
                painter.setPen(QColor("#0284c7"));
                painter.drawRect(pt.x() - 4, pt.y() - 4, 8, 8);
            };

            drawHandle(selRect.topLeft());
            drawHandle(QPoint(selRect.center().x(), selRect.top()));
            drawHandle(selRect.topRight());
            drawHandle(QPoint(selRect.left(), selRect.center().y()));
            drawHandle(QPoint(selRect.right(), selRect.center().y()));
            drawHandle(selRect.bottomLeft());
            drawHandle(QPoint(selRect.center().x(), selRect.bottom()));
            drawHandle(selRect.bottomRight());
        }
    }
}

void PreviewWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier))) {
        isPanning_ = true;
        lastMousePos_ = event->pos();
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton) {
        const auto& sel = engine_.selectedClips();
        auto* tl = engine_.activeTimeline();
        if (!sel.empty() && tl) {
            const auto* clip = tl->findClip(sel[0]);
            if (clip) {
                int canvasW = engine_.project().settings().canvasSize.width;
                int canvasH = engine_.project().settings().canvasSize.height;
                if (canvasW <= 0) canvasW = 1920;
                if (canvasH <= 0) canvasH = 1080;

                double scale = (zoomFactor_ < 0.0) ? std::max(0.05, std::min(static_cast<double>(width() - 48) / canvasW, static_cast<double>(height() - 48) / canvasH)) : zoomFactor_;
                int dispW = static_cast<int>(canvasW * scale);
                int dispH = static_cast<int>(canvasH * scale);
                int startX = (width() - dispW) / 2 + panOffset_.x();
                int startY = (height() - dispH) / 2 + panOffset_.y();

                double px = clip->getParam<double>("transform.positionX", 0.0);
                double py = clip->getParam<double>("transform.positionY", 0.0);
                double sx = clip->getParam<double>("transform.scaleX", 1.0);
                double sy = clip->getParam<double>("transform.scaleY", 1.0);

                int boxW = static_cast<int>(dispW * std::max(0.2, sx * 0.8));
                int boxH = static_cast<int>(dispH * std::max(0.2, sy * 0.8));
                if (clip->type() == editor::ClipType::Text || clip->type() == editor::ClipType::Graphic) {
                    boxW = static_cast<int>(240 * scale * sx);
                    boxH = static_cast<int>(100 * scale * sy);
                }

                int boxX = startX + dispW / 2 + static_cast<int>(px * scale) - boxW / 2;
                int boxY = startY + dispH / 2 + static_cast<int>(py * scale) - boxH / 2;
                QRect selRect(boxX, boxY, boxW, boxH);

                HandleType ht = hitTestHandles(event->pos(), selRect);
                if (ht != HandleType::None) {
                    activeHandle_ = ht;
                    activeClipId_ = clip->id();
                    dragStartPos_ = event->pos();
                    origPosX_ = px;
                    origPosY_ = py;
                    origScaleX_ = sx;
                    origScaleY_ = sy;
                    origRotate_ = clip->getParam<double>("transform.rotate", 0.0);
                    event->accept();
                    return;
                }
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void PreviewWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isPanning_) {
        QPoint delta = event->pos() - lastMousePos_;
        panOffset_ += delta;
        lastMousePos_ = event->pos();
        update();
        event->accept();
        return;
    }

    if (activeHandle_ != HandleType::None) {
        auto* tl = engine_.activeTimeline();
        if (!tl) return;
        auto* clip = tl->findClip(activeClipId_);
        if (!clip) return;

        int canvasW = engine_.project().settings().canvasSize.width;
        int canvasH = engine_.project().settings().canvasSize.height;
        if (canvasW <= 0) canvasW = 1920;
        if (canvasH <= 0) canvasH = 1080;

        double scale = (zoomFactor_ < 0.0) ? std::max(0.05, std::min(static_cast<double>(width() - 48) / canvasW, static_cast<double>(height() - 48) / canvasH)) : zoomFactor_;
        QPoint delta = event->pos() - dragStartPos_;

        if (activeHandle_ == HandleType::Move) {
            double newX = origPosX_ + delta.x() / scale;
            double newY = origPosY_ + delta.y() / scale;

            // Snapping to center
            showCenterGuideX_ = (std::abs(newX) < 8.0);
            if (showCenterGuideX_) newX = 0.0;

            showCenterGuideY_ = (std::abs(newY) < 8.0);
            if (showCenterGuideY_) newY = 0.0;

            clip->setParam("transform.positionX", newX);
            clip->setParam("transform.positionY", newY);
        } else if (activeHandle_ == HandleType::TopLeft || activeHandle_ == HandleType::TopRight ||
                   activeHandle_ == HandleType::BotLeft || activeHandle_ == HandleType::BotRight) {
            double deltaScale = static_cast<double>(delta.x() + delta.y()) / 200.0;
            double newScale = std::clamp(origScaleX_ + deltaScale, 0.05, 10.0);
            clip->setParam("transform.scaleX", newScale);
            clip->setParam("transform.scaleY", newScale);
        } else if (activeHandle_ == HandleType::Rotate) {
            double angle = origRotate_ + delta.x() * 0.5;
            clip->setParam("transform.rotate", angle);
        }

        engine_.project().setDirty(true);
        update();
        event->accept();
        return;
    }

    QWidget::mouseMoveEvent(event);
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (isPanning_) {
        isPanning_ = false;
        event->accept();
        return;
    }

    if (activeHandle_ != HandleType::None) {
        activeHandle_ = HandleType::None;
        showCenterGuideX_ = false;
        showCenterGuideY_ = false;
        update();
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void PreviewWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        double delta = event->angleDelta().y();
        double factor = (delta > 0) ? 1.15 : (1.0 / 1.15);
        if (zoomFactor_ < 0.0) {
            zoomFactor_ = 1.0;
        }
        zoomFactor_ = std::clamp(zoomFactor_ * factor, 0.05, 10.0);
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

} // namespace catchim::ui
#endif
