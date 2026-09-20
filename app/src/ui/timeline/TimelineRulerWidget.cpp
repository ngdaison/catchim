#include "TimelineRulerWidget.h"

#if defined(HAVE_QT6)
#include "ui/theme/Theme.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

namespace catchim::ui {

TimelineRulerWidget::TimelineRulerWidget(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setFixedHeight(Metrics::timelineRulerHeight + Metrics::timelineBookmarkHeight);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

core::TimelineTime TimelineRulerWidget::pixelToTime(int pixelX) const {
    double pixelsPerSecond = 50.0 * zoomFactor_;
    double sec = static_cast<double>(pixelX + scrollOffset_) / pixelsPerSecond;
    return core::TimelineTime::fromSeconds(std::max(0.0, sec));
}

void TimelineRulerWidget::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#141414"));

    double pixelsPerSecond = 50.0 * zoomFactor_;
    int w = width();

    // Draw bottom border
    painter.setPen(QColor("#292929"));
    painter.drawLine(0, height() - 1, w, height() - 1);

    // Draw ruler tick marks
    painter.setPen(QColor("#808080"));
    painter.setFont(QFont("Inter", 8));

    double startSec = std::max(0.0, static_cast<double>(scrollOffset_) / pixelsPerSecond);
    double endSec = static_cast<double>(scrollOffset_ + w) / pixelsPerSecond;

    // Step size depends on zoom
    double stepSec = 1.0;
    if (pixelsPerSecond < 20.0) stepSec = 5.0;
    else if (pixelsPerSecond > 200.0) stepSec = 0.5;

    int firstTick = static_cast<int>(std::floor(startSec / stepSec));
    int lastTick = static_cast<int>(std::ceil(endSec / stepSec));

    for (int i = firstTick; i <= lastTick; ++i) {
        double sec = i * stepSec;
        int x = static_cast<int>(sec * pixelsPerSecond) - scrollOffset_;
        if (x < 0 || x >= w) continue;

        // Major tick
        painter.drawLine(x, 10, x, Metrics::timelineRulerHeight);

        // Time label (e.g. 0s, 1s, 2s, or 00:01)
        int minutes = static_cast<int>(sec) / 60;
        int seconds = static_cast<int>(sec) % 60;
        QString text = (minutes > 0)
            ? QString("%1:%2").arg(minutes).arg(seconds, 2, 10, QChar('0'))
            : QString("%1s").arg(seconds);

        painter.drawText(x + 3, 16, text);
    }

    // Draw Bookmarks Row (height 16px at bottom)
    auto* tl = engine_.activeTimeline();
    if (tl) {
        int bmY = Metrics::timelineRulerHeight + 2;
        for (const auto& bm : tl->bookmarks()) {
            double sec = bm.time.toSeconds();
            int x = static_cast<int>(sec * pixelsPerSecond) - scrollOffset_;
            if (x >= -8 && x < w + 8) {
                // Draw small diamond bookmark marker
                painter.setBrush(QColor("#009DFF"));
                painter.setPen(Qt::NoPen);
                QPolygon poly;
                poly << QPoint(x, bmY)
                     << QPoint(x + 4, bmY + 5)
                     << QPoint(x, bmY + 10)
                     << QPoint(x - 4, bmY + 5);
                painter.drawPolygon(poly);
            }
        }
    }

    // Draw Playhead cursor handle on ruler
    core::TimelineTime cur = engine_.playback().currentTime();
    int playheadX = static_cast<int>(cur.toSeconds() * pixelsPerSecond) - scrollOffset_;
    if (playheadX >= -5 && playheadX < w + 5) {
        painter.setBrush(QColor("#16A9F3"));
        painter.setPen(Qt::NoPen);
        QPolygon head;
        head << QPoint(playheadX - 5, 0)
             << QPoint(playheadX + 5, 0)
             << QPoint(playheadX + 5, Metrics::timelineRulerHeight - 4)
             << QPoint(playheadX, Metrics::timelineRulerHeight + 2)
             << QPoint(playheadX - 5, Metrics::timelineRulerHeight - 4);
        painter.drawPolygon(head);
    }
}

void TimelineRulerWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDragging_ = true;
        core::TimelineTime time = pixelToTime(event->pos().x());
        emit seekRequested(time);
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void TimelineRulerWidget::mouseMoveEvent(QMouseEvent* event) {
    if (isDragging_) {
        core::TimelineTime time = pixelToTime(event->pos().x());
        emit seekRequested(time);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void TimelineRulerWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (isDragging_ && event->button() == Qt::LeftButton) {
        isDragging_ = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

} // namespace catchim::ui
#endif
