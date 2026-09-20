#include "TimelineTracksWidget.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include "ui/theme/Theme.h"
#include "media/waveform/WaveformGenerator.h"
#include "media/probe/MediaProbe.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
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
    setAcceptDrops(true);
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
    if (!tl) return res;

    int yOffset = Metrics::timelineContentTopPadding - scrollY_;

    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        if (pos.y() >= yOffset && pos.y() < yOffset + trackHeight) {
            res.track = track;

            if (pos.x() < Metrics::trackLabelsWidth) {
                // Header buttons
                if (pos.x() >= Metrics::trackLabelsWidth - 40 && pos.x() < Metrics::trackLabelsWidth - 20) {
                    res.isMuteBtn = true;
                } else if (pos.x() >= Metrics::trackLabelsWidth - 20) {
                    res.isHideBtn = true;
                }
                return res;
            }

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
    painter.fillRect(rect(), QColor("#0c0c0e"));

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    int yOffset = Metrics::timelineContentTopPadding - scrollY_;

    // 1. Draw track rows
    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        // Track lane background
        QRect trackRect(Metrics::trackLabelsWidth, yOffset, width() - Metrics::trackLabelsWidth, trackHeight);
        painter.fillRect(trackRect, QColor("#141417"));
        painter.setPen(QColor("#1f1f23"));
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
                case editor::ClipType::Image: clipBg = palette.clipVideo.lighter(115); break;
                case editor::ClipType::Sticker: clipBg = palette.clipGraphic; break;
            }

            // Fill clip body
            painter.setBrush(clipBg);
            painter.setPen(isSelected ? QPen(palette.primaryAccent, 2.0) : QPen(QColor(0, 0, 0, 100), 1.0));
            painter.drawRoundedRect(clipRect.adjusted(1, 1, -1, -1), 6, 6);

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
                        int h1 = static_cast<int>(wf->buckets[b].maxPeak * (trackHeight / 2 - 6));
                        int h2 = static_cast<int>(wf->buckets[b].minPeak * (trackHeight / 2 - 6));
                        painter.drawLine(bx, midY - h1, bx, midY - h2);
                    }
                }
            }

            // Clip label text
            painter.setPen(QColor("#FFFFFF"));
            painter.setFont(QFont("Inter", 8, QFont::Bold));
            QString label = QString::fromStdString(clip.name());
            painter.drawText(clipRect.adjusted(8, 6, -8, -6), Qt::AlignLeft | Qt::AlignTop, label);
        }

        yOffset += trackHeight + Metrics::trackGap;
    }

    // 2. Draw Track Labels Column on the left (112px, fixed overlay)
    QRect labelsCol(0, 0, Metrics::trackLabelsWidth, height());
    painter.fillRect(labelsCol, QColor("#111114"));
    painter.setPen(QColor("#27272a"));
    painter.drawLine(Metrics::trackLabelsWidth - 1, 0, Metrics::trackLabelsWidth - 1, height());

    yOffset = Metrics::timelineContentTopPadding - scrollY_;
    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        QRect labelCell(0, yOffset, Metrics::trackLabelsWidth - 1, trackHeight);
        painter.setPen(QColor("#1f1f23"));
        painter.drawLine(labelCell.bottomLeft(), labelCell.bottomRight());

        // Track Icon & Name
        painter.setFont(QFont("Inter", 8, QFont::DemiBold));
        painter.setPen(QColor("#f4f4f5"));

        UiIcon tIcon = UiIcon::Media;
        if (track->type() == editor::TrackType::Audio) tIcon = UiIcon::Audio;
        else if (track->type() == editor::TrackType::Text) tIcon = UiIcon::Text;
        else if (track->type() == editor::TrackType::Graphic) tIcon = UiIcon::Stickers;
        else if (track->type() == editor::TrackType::Effect) tIcon = UiIcon::Effects;

        int iconY = yOffset + (trackHeight - 14) / 2;
        painter.drawPixmap(8, iconY, UiIcons::getPixmap(tIcon, QColor("#a1a1aa"), 14));
        painter.drawText(26, yOffset + (trackHeight + 8) / 2, QString::fromStdString(track->name()));

        // Mute / Visibility indicators (Vector SVG)
        UiIcon muteIcon = track->isMuted() ? UiIcon::VolumeMute : UiIcon::Volume;
        QColor muteCol = track->isMuted() ? palette.destructive : QColor("#71717a");
        painter.drawPixmap(Metrics::trackLabelsWidth - 38, iconY, UiIcons::getPixmap(muteIcon, muteCol, 14));

        UiIcon eyeIcon = track->isHidden() ? UiIcon::EyeOff : UiIcon::Eye;
        QColor eyeCol = track->isHidden() ? palette.destructive : QColor("#71717a");
        painter.drawPixmap(Metrics::trackLabelsWidth - 20, iconY, UiIcons::getPixmap(eyeIcon, eyeCol, 14));

        yOffset += trackHeight + Metrics::trackGap;
    }

    // 3. Draw Playhead Vertical Line across all tracks
    int playheadX = timeToPixel(engine_.playback().currentTime());
    if (playheadX >= Metrics::trackLabelsWidth && playheadX < width()) {
        painter.setPen(QPen(palette.primaryAccent, 2.0));
        painter.drawLine(playheadX, 0, playheadX, height());
    }
}

void TimelineTracksWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        auto hit = hitTest(event->pos());
        if (hit.track && (hit.isMuteBtn || hit.isHideBtn)) {
            auto* track = const_cast<editor::Track*>(hit.track);
            if (hit.isMuteBtn) {
                track->setMuted(!track->isMuted());
            } else if (hit.isHideBtn) {
                track->setHidden(!track->isHidden());
            }
            engine_.project().setDirty(true);
            update();
            event->accept();
            return;
        }

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
            // Click on empty space -> Seek
            engine_.deselectAll();
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
        auto hit = hitTest(event->pos());
        if (hit.isTrimStart || hit.isTrimEnd) {
            setCursor(Qt::SizeHorCursor);
        } else if (hit.isMuteBtn || hit.isHideBtn) {
            setCursor(Qt::PointingHandCursor);
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

void TimelineTracksWidget::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void TimelineTracksWidget::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void TimelineTracksWidget::dropEvent(QDropEvent* event) {
    const auto* mime = event->mimeData();
    if (!mime->hasUrls()) return;

    core::TimelineTime dropTime = pixelToTime(event->position().toPoint().x());
    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    for (const auto& url : mime->urls()) {
        QString localPath = url.toLocalFile();
        if (localPath.isEmpty()) continue;

        auto probeRes = media::MediaProbe::probe(localPath.toStdString());
        if (probeRes.ok()) {
            auto asset = probeRes.unwrap();
            mediaLibrary_.addAsset(asset);

            core::TrackId targetTrack = tl->mainTrack().id();
            if (asset->type() == media::MediaType::Audio) {
                for (const auto* t : tl->allTracks()) {
                    if (t->type() == editor::TrackType::Audio) {
                        targetTrack = t->id();
                        break;
                    }
                }
            }

            editor::Clip clip(
                core::ClipId::generate(),
                (asset->type() == media::MediaType::Video) ? editor::ClipType::Video :
                (asset->type() == media::MediaType::Audio) ? editor::ClipType::Audio : editor::ClipType::Image,
                asset->fileName(),
                dropTime,
                asset->duration()
            );
            clip.setMediaId(asset->id());
            engine_.addClip(targetTrack, std::move(clip));

            dropTime = dropTime + asset->duration();
        }
    }

    event->acceptProposedAction();
    update();
}

} // namespace catchim::ui
#endif
