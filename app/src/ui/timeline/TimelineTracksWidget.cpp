#include "TimelineTracksWidget.h"

#if defined(HAVE_QT6)
#include "ui/theme/Theme.h"
#include "media/waveform/WaveformGenerator.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

namespace catchim::ui {

TimelineTracksWidget::TimelineTracksWidget(
    editor::EditorEngine& engine,
    media::MediaLibrary& mediaLibrary,
    QWidget* parent
)
    : QWidget(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

int TimelineTracksWidget::totalTracksHeight() const {
    auto* tl = engine_.activeTimeline();
    if (!tl) return 200;

    int total = Metrics::timelineContentTopPadding;
    for (const auto* track : tl->allTracks()) {
        int h = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) h = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) h = Metrics::trackHeightText;
        total += h + Metrics::trackGap;
    }
    return std::max(total, 200);
}

core::TimelineTime TimelineTracksWidget::pixelToTime(int pixelX) const {
    int contentX = pixelX - Metrics::trackLabelsWidth + scrollX_;
    double pixelsPerSecond = 50.0 * zoomFactor_;
    double sec = static_cast<double>(contentX) / pixelsPerSecond;
    return core::TimelineTime::fromSeconds(std::max(0.0, sec));
}

int TimelineTracksWidget::timeToPixel(core::TimelineTime time) const {
    double pixelsPerSecond = 50.0 * zoomFactor_;
    return static_cast<int>(time.toSeconds() * pixelsPerSecond) - scrollX_ + Metrics::trackLabelsWidth;
}

TimelineTracksWidget::HitTestResult TimelineTracksWidget::hitTest(const QPoint& pos) const {
    HitTestResult res;
    auto* tl = engine_.activeTimeline();
    if (!tl || pos.x() < Metrics::trackLabelsWidth) return res;

    int yOffset = Metrics::timelineContentTopPadding - scrollY_;

    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        if (pos.y() >= yOffset && pos.y() < yOffset + trackHeight) {
            res.track = track;
            // Check clips
            for (const auto& clip : track->clips()) {
                int clipLeft = timeToPixel(clip.startTime());
                int clipRight = timeToPixel(clip.endTime());

                if (pos.x() >= clipLeft && pos.x() <= clipRight) {
                    res.clip = &clip;
                    if (std::abs(pos.x() - clipLeft) <= 6) {
                        res.isTrimStart = true;
                    } else if (std::abs(pos.x() - clipRight) <= 6) {
                        res.isTrimEnd = true;
                    }
                    return res;
                }
            }
            return res;
        }
        yOffset += trackHeight + Metrics::trackGap;
    }
    return res;
}

void TimelineTracksWidget::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);
    const auto& palette = Theme::instance().palette();

    // Background tracks container
    painter.fillRect(rect(), QColor("#111111"));

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    int yOffset = Metrics::timelineContentTopPadding - scrollY_;
    double pixelsPerSecond = 50.0 * zoomFactor_;

    // 1. Draw track rows
    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        // Track lane background
        QRect trackRect(Metrics::trackLabelsWidth, yOffset, width() - Metrics::trackLabelsWidth, trackHeight);
        painter.fillRect(trackRect, QColor("#161616"));
        painter.setPen(QColor("#202020"));
        painter.drawLine(trackRect.bottomLeft(), trackRect.bottomRight());

        // Draw clips on this track
        for (const auto& clip : track->clips()) {
            int clipX = timeToPixel(clip.startTime());
            int clipW = std::max(4, timeToPixel(clip.endTime()) - clipX);
            QRect clipRect(clipX, yOffset, clipW, trackHeight);

            bool isSelected = engine_.isClipSelected(clip.id());

            // Base color
            QColor clipBg;
            switch (clip.type()) {
                case editor::ClipType::Video: clipBg = palette.clipVideo; break;
                case editor::ClipType::Audio: clipBg = palette.clipAudio; break;
                case editor::ClipType::Text: clipBg = palette.clipText; break;
                case editor::ClipType::Graphic: clipBg = palette.clipGraphic; break;
                case editor::ClipType::Effect: clipBg = palette.clipEffect; break;
                case editor::ClipType::Image: clipBg = palette.clipVideo.lighter(110); break;
                case editor::ClipType::Sticker: clipBg = palette.clipGraphic; break;
            }

            // Fill clip body
            painter.setBrush(clipBg);
            painter.setPen(isSelected ? QPen(palette.primaryAccent, 2.0) : QPen(QColor(0, 0, 0, 80), 1.0));
            painter.drawRoundedRect(clipRect.adjusted(1, 1, -1, -1), 4, 4);

            // Draw Audio Waveform if audio clip
            if (clip.type() == editor::ClipType::Audio) {
                auto wf = media::WaveformGenerator::instance().getWaveform(clip.mediaId());
                if (!wf) {
                    wf = media::WaveformGenerator::generateDummyWaveform(clip.mediaId(), std::max(10, clipW / 3));
                }
                if (wf && !wf->buckets.empty()) {
                    painter.setPen(palette.waveformColor);
                    int midY = yOffset + trackHeight / 2;
                    size_t count = std::min<size_t>(wf->buckets.size(), clipW / 2);
                    for (size_t b = 0; b < count; ++b) {
                        int bx = clipX + static_cast<int>(b * 2);
                        int h1 = static_cast<int>(wf->buckets[b].maxPeak * (trackHeight / 2 - 4));
                        int h2 = static_cast<int>(wf->buckets[b].minPeak * (trackHeight / 2 - 4));
                        painter.drawLine(bx, midY - h1, bx, midY - h2);
                    }
                }
            }

            // Clip label text
            painter.setPen(QColor("#FFFFFF"));
            painter.setFont(QFont("Inter", 8, QFont::Bold));
            QString label = QString::fromStdString(clip.name());
            painter.drawText(clipRect.adjusted(6, 4, -6, -4), Qt::AlignLeft | Qt::AlignTop, label);
        }

        yOffset += trackHeight + Metrics::trackGap;
    }

    // 2. Draw Track Labels Column on the left (112px, fixed overlay)
    QRect labelsCol(0, 0, Metrics::trackLabelsWidth, height());
    painter.fillRect(labelsCol, QColor("#141414"));
    painter.setPen(QColor("#292929"));
    painter.drawLine(Metrics::trackLabelsWidth - 1, 0, Metrics::trackLabelsWidth - 1, height());

    yOffset = Metrics::timelineContentTopPadding - scrollY_;
    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        QRect labelCell(0, yOffset, Metrics::trackLabelsWidth - 1, trackHeight);
        painter.setPen(QColor("#202020"));
        painter.drawLine(labelCell.bottomLeft(), labelCell.bottomRight());

        // Track Icon & Name
        painter.setFont(QFont("Inter", 8));
        painter.setPen(QColor("#A0A0A0"));
        QString typeIcon = "📹";
        if (track->type() == editor::TrackType::Audio) typeIcon = "🎵";
        else if (track->type() == editor::TrackType::Text) typeIcon = "🆃";
        else if (track->type() == editor::TrackType::Graphic) typeIcon = "◨";
        else if (track->type() == editor::TrackType::Effect) typeIcon = "✨";

        painter.drawText(8, yOffset + 16, typeIcon);
        painter.drawText(28, yOffset + 16, QString::fromStdString(track->name()));

        // Mute / Visibility indicators
        if (track->isMuted()) {
            painter.setPen(palette.destructive);
            painter.drawText(Metrics::trackLabelsWidth - 32, yOffset + 16, "🔇");
        }
        if (track->isHidden()) {
            painter.setPen(palette.destructive);
            painter.drawText(Metrics::trackLabelsWidth - 16, yOffset + 16, "👁");
        }

        yOffset += trackHeight + Metrics::trackGap;
    }

    // 3. Draw Playhead Vertical Line across all tracks
    int playheadX = timeToPixel(engine_.playback().currentTime());
    if (playheadX >= Metrics::trackLabelsWidth && playheadX < width()) {
        painter.setPen(QPen(palette.primaryAccent, 1.5));
        painter.drawLine(playheadX, 0, playheadX, height());
    }
}

void TimelineTracksWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        auto hit = hitTest(event->pos());
        if (hit.clip) {
            activeClipId_ = hit.clip->id();
            dragStartPos_ = event->pos();
            dragStartTime_ = hit.clip->startTime();
            dragStartDuration_ = hit.clip->duration();

            if (hit.isTrimStart) {
                dragMode_ = DragMode::TrimmingStart;
            } else if (hit.isTrimEnd) {
                dragMode_ = DragMode::TrimmingEnd;
            } else {
                dragMode_ = DragMode::MovingClip;
                bool additive = (event->modifiers() & Qt::ShiftModifier) || (event->modifiers() & Qt::ControlModifier);
                engine_.selectClip(hit.clip->id(), additive);
                emit clipSelected(hit.clip->id());
            }
            update();
            event->accept();
            return;
        } else {
            // Click on empty space
            engine_.deselectAll();
            // Seek playhead to click position
            core::TimelineTime clickTime = pixelToTime(event->pos().x());
            emit seekRequested(clickTime);
            update();
            event->accept();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void TimelineTracksWidget::mouseMoveEvent(QMouseEvent* event) {
    if (dragMode_ == DragMode::None) {
        // Update cursor for trim handles
        auto hit = hitTest(event->pos());
        if (hit.isTrimStart || hit.isTrimEnd) {
            setCursor(Qt::SizeHorCursor);
        } else if (hit.clip) {
            setCursor(Qt::ArrowCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else if (dragMode_ == DragMode::MovingClip) {
        int deltaX = event->pos().x() - dragStartPos_.x();
        double pixelsPerSecond = 50.0 * zoomFactor_;
        core::TimelineTime deltaTime = core::TimelineTime::fromSeconds(deltaX / pixelsPerSecond);
        core::TimelineTime newStart = dragStartTime_ + deltaTime;
        if (newStart.ticks() >= 0) {
            auto* track = engine_.activeTimeline()->findTrackContainingClip(activeClipId_);
            if (track) {
                engine_.moveClip(activeClipId_, track->id(), newStart);
                update();
            }
        }
    } else if (dragMode_ == DragMode::TrimmingStart) {
        core::TimelineTime targetTime = pixelToTime(event->pos().x());
        engine_.trimClipStart(activeClipId_, targetTime);
        update();
    } else if (dragMode_ == DragMode::TrimmingEnd) {
        core::TimelineTime targetTime = pixelToTime(event->pos().x());
        core::TimelineTime newDur = targetTime - dragStartTime_;
        if (newDur.ticks() > 0) {
            engine_.trimClipEnd(activeClipId_, newDur);
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TimelineTracksWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (dragMode_ != DragMode::None) {
        dragMode_ = DragMode::None;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

} // namespace catchim::ui
#endif
