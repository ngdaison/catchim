#include "PreviewWidget.h"

#if defined(HAVE_QT6)
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

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
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void PreviewWidget::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#0D0D0D")); // Background viewport

    // Render frame at current playhead time
    core::TimelineTime curTime = engine_.playback().currentTime();
    const auto& frame = renderEngine_.renderFrame(engine_.project(), mediaLibrary_, curTime);

    if (frame.width <= 0 || frame.height <= 0 || frame.rgbaPixels.empty()) {
        return;
    }

    QImage img(
        frame.rgbaPixels.data(),
        frame.width,
        frame.height,
        frame.width * 4,
        QImage::Format_RGBA8888
    );

    // Compute display dimensions
    double scale = 1.0;
    if (zoomFactor_ < 0.0) {
        // Auto fit
        double scaleW = static_cast<double>(width() - 32) / frame.width;
        double scaleH = static_cast<double>(height() - 32) / frame.height;
        scale = std::max(0.1, std::min(scaleW, scaleH));
    } else {
        scale = zoomFactor_;
    }

    int dispW = static_cast<int>(frame.width * scale);
    int dispH = static_cast<int>(frame.height * scale);
    int startX = (width() - dispW) / 2 + panOffset_.x();
    int startY = (height() - dispH) / 2 + panOffset_.y();

    // Draw canvas border
    painter.setPen(QColor("#292929"));
    painter.drawRect(startX - 1, startY - 1, dispW + 2, dispH + 2);

    // Draw image
    painter.drawImage(QRect(startX, startY, dispW, dispH), img);
}

void PreviewWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton || (event->button() == Qt::LeftButton && (event->modifiers() & Qt::AltModifier))) {
        isPanning_ = true;
        lastMousePos_ = event->pos();
        event->accept();
        return;
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
    QWidget::mouseMoveEvent(event);
}

void PreviewWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (isPanning_ && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        isPanning_ = false;
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
        zoomFactor_ = std::clamp(zoomFactor_ * factor, 0.1, 5.0);
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

} // namespace catchim::ui
#endif
