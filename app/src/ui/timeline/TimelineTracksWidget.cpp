#include "TimelineTracksWidget.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include "ui/theme/Theme.h"
#include "media/waveform/WaveformGenerator.h"
#include "media/probe/MediaProbe.h"
#include "audio/AudioWaveformBarEngine.h"
#include "audio/AudioVolumeLineEngine.h"
#include "editor/timeline/AdvancedSnapEngine.h"
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

QSize TimelineTracksWidget::sizeHint() const {
    double pixelsPerSecond = 50.0 * zoomFactor_;
    double durSec = engine_.project().totalDuration().toSeconds();
    if (durSec < 30.0) durSec = 30.0;
    int w = Metrics::trackLabelsWidth + static_cast<int>(durSec * pixelsPerSecond) + 300;
    return QSize(w, totalTracksHeight());
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
    painter.fillRect(rect(), palette.background);

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
        painter.fillRect(trackRect, palette.panelBackground);
        painter.setPen(palette.border);
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

            // Draw Audio Waveform & Volume line if audio clip
            if (clip.type() == editor::ClipType::Audio) {
                auto wf = media::WaveformGenerator::instance().getWaveform(clip.mediaId());
                if (!wf) {
                    wf = media::WaveformGenerator::generateDummyWaveform(clip.mediaId(), std::max(10, clipW / 2));
                }
                double volLinear = clip.getParam<double>("volume", 1.0);
                std::vector<double> gainSamples = { volLinear };

                if (wf && !wf->buckets.empty()) {
                    std::vector<float> amplitudes;
                    amplitudes.reserve(wf->buckets.size());
                    for (const auto& b : wf->buckets) {
                        amplitudes.push_back(b.maxPeak);
                    }

                    double innerH = static_cast<double>(trackHeight - 12);
                    auto bars = audio::AudioWaveformBarEngine::calculateWaveformBars(
                        amplitudes,
                        static_cast<double>(clipW),
                        innerH,
                        gainSamples,
                        clip.duration().toSeconds()
                    );

                    for (const auto& bar : bars) {
                        int bx = clipX + static_cast<int>(bar.x);
                        int by = yOffset + 6 + static_cast<int>(bar.top);
                        int bh = std::max(1, static_cast<int>(bar.height));
                        painter.setPen(bar.isBurnt ? QColor("#ff6e14") : palette.waveformColor);
                        painter.drawLine(bx, by, bx, by + bh);
                    }
                }

                // Draw horizontal volume automation line
                double volDb = (volLinear > 0.0001) ? (20.0 * std::log10(volLinear)) : -60.0;
                double linePosPercent = audio::AudioVolumeLineEngine::getLinePositionPercent(volDb);
                int lineY = yOffset + 6 + static_cast<int>((linePosPercent / 100.0) * (trackHeight - 12));
                QColor volLineColor = (Theme::instance().mode() == ThemeMode::Dark)
                    ? QColor(255, 255, 255, 130) : QColor(0, 0, 0, 130);
                painter.setPen(QPen(volLineColor, 1.0, Qt::DashLine));
                painter.drawLine(clipX, lineY, clipX + clipW, lineY);
            }

            // Clip label text
            if (clipRect.width() > 14) {
                painter.save();
                painter.setClipRect(clipRect.adjusted(2, 2, -2, -2));
                painter.setPen(QColor("#FFFFFF"));
                QFont font("Inter", 8, QFont::Bold);
                painter.setFont(font);
                QFontMetrics fm(font);
                int availW = clipRect.width() - 16;
                if (availW > 8) {
                    QString label = fm.elidedText(QString::fromStdString(clip.name()), Qt::ElideRight, availW);
                    painter.drawText(clipRect.adjusted(8, 6, -8, -6), Qt::AlignLeft | Qt::AlignTop, label);
                }
                painter.restore();
            }
        }

        yOffset += trackHeight + Metrics::trackGap;
    }

    // 2. Draw Track Labels Column on the left (112px, fixed overlay)
    QRect labelsCol(0, 0, Metrics::trackLabelsWidth, height());
    painter.fillRect(labelsCol, palette.secondary);
    painter.setPen(palette.border);
    painter.drawLine(Metrics::trackLabelsWidth - 1, 0, Metrics::trackLabelsWidth - 1, height());

    yOffset = Metrics::timelineContentTopPadding - scrollY_;
    for (const auto* track : tl->allTracks()) {
        int trackHeight = Metrics::trackHeightVideo;
        if (track->type() == editor::TrackType::Audio) trackHeight = Metrics::trackHeightAudio;
        else if (track->type() != editor::TrackType::Video) trackHeight = Metrics::trackHeightText;

        QRect labelCell(0, yOffset, Metrics::trackLabelsWidth - 1, trackHeight);
        painter.setPen(palette.border);
        painter.drawLine(labelCell.bottomLeft(), labelCell.bottomRight());

        // Track Icon & Name
        painter.setFont(QFont("Inter", 8, QFont::DemiBold));
        painter.setPen(palette.textPrimary);

        UiIcon tIcon = UiIcon::Media;
        if (track->type() == editor::TrackType::Audio) tIcon = UiIcon::Audio;
        else if (track->type() == editor::TrackType::Text) tIcon = UiIcon::Text;
        else if (track->type() == editor::TrackType::Graphic) tIcon = UiIcon::Stickers;
        else if (track->type() == editor::TrackType::Effect) tIcon = UiIcon::Effects;

        int iconY = yOffset + (trackHeight - 14) / 2;
        painter.drawPixmap(8, iconY, UiIcons::getPixmap(tIcon, palette.textSecondary, 14));
        painter.drawText(26, yOffset + (trackHeight + 8) / 2, QString::fromStdString(track->name()));

        // Mute / Visibility indicators (Vector SVG)
        UiIcon muteIcon = track->isMuted() ? UiIcon::VolumeMute : UiIcon::Volume;
        QColor muteCol = track->isMuted() ? palette.destructive : palette.textSecondary;
        painter.drawPixmap(Metrics::trackLabelsWidth - 38, iconY, UiIcons::getPixmap(muteIcon, muteCol, 14));

        UiIcon eyeIcon = track->isHidden() ? UiIcon::EyeOff : UiIcon::Eye;
        QColor eyeCol = track->isHidden() ? palette.destructive : palette.textSecondary;
        painter.drawPixmap(Metrics::trackLabelsWidth - 20, iconY, UiIcons::getPixmap(eyeIcon, eyeCol, 14));

        yOffset += trackHeight + Metrics::trackGap;
    }

    // 3. Draw Playhead Vertical Line across all tracks
    int playheadX = timeToPixel(engine_.playback().currentTime());
    if (playheadX >= Metrics::trackLabelsWidth && playheadX < width()) {
        painter.setPen(QPen(palette.primaryAccent, 2.0));
        painter.drawLine(playheadX, 0, playheadX, height());
    }

    // 4. Draw Snap Indicator Guideline if snapping is active
    if (snapIndicatorX_ >= Metrics::trackLabelsWidth && snapIndicatorX_ < width()) {
        painter.setPen(QPen(palette.primaryAccent, 1.5, Qt::DashLine));
        painter.drawLine(snapIndicatorX_, 0, snapIndicatorX_, height());
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
            auto* tl = engine_.activeTimeline();
            if (tl) {
                auto* track = tl->findTrackContainingClip(activeClipId_);
                if (track) {
                    if (engine_.isSnappingEnabled()) {
                        auto snapPoints = editor::AdvancedSnapEngine::collectAllSnapPoints(*tl, engine_.playback().currentTime(), true, activeClipId_);
                        auto maxSnap = editor::AdvancedSnapEngine::getTimelineSnapThresholdInTicks(zoomFactor_, 8.0);

                        auto snapStart = editor::AdvancedSnapEngine::resolveSortedTimelineSnap(newStart, snapPoints, maxSnap);
                        if (snapStart.hasSnapped) {
                            newStart = snapStart.snappedTime;
                            snapIndicatorX_ = timeToPixel(newStart);
                        } else {
                            auto snapEnd = editor::AdvancedSnapEngine::resolveSortedTimelineSnap(newStart + dragStartDuration_, snapPoints, maxSnap);
                            if (snapEnd.hasSnapped) {
                                newStart = snapEnd.snappedTime - dragStartDuration_;
                                snapIndicatorX_ = timeToPixel(snapEnd.snappedTime);
                            } else {
                                snapIndicatorX_ = -1;
                            }
                        }
                    } else {
                        snapIndicatorX_ = -1;
                    }

                    if (newStart.ticks() >= 0) {
                        engine_.moveClip(activeClipId_, track->id(), newStart);
                    }
                    update();
                }
            }
        }
    } else if (dragMode_ == DragMode::TrimmingStart) {
        core::TimelineTime targetTime = pixelToTime(event->pos().x());
        auto* tl = engine_.activeTimeline();
        if (tl && engine_.isSnappingEnabled()) {
            auto snapPoints = editor::AdvancedSnapEngine::collectAllSnapPoints(*tl, engine_.playback().currentTime(), true, activeClipId_);
            auto maxSnap = editor::AdvancedSnapEngine::getTimelineSnapThresholdInTicks(zoomFactor_, 8.0);
            auto snapRes = editor::AdvancedSnapEngine::resolveSortedTimelineSnap(targetTime, snapPoints, maxSnap);
            if (snapRes.hasSnapped) {
                targetTime = snapRes.snappedTime;
                snapIndicatorX_ = timeToPixel(targetTime);
            } else {
                snapIndicatorX_ = -1;
            }
        } else {
            snapIndicatorX_ = -1;
        }
        engine_.trimClipStart(activeClipId_, targetTime);
        update();
    } else if (dragMode_ == DragMode::TrimmingEnd) {
        core::TimelineTime targetTime = pixelToTime(event->pos().x());
        auto* tl = engine_.activeTimeline();
        if (tl && engine_.isSnappingEnabled()) {
            auto snapPoints = editor::AdvancedSnapEngine::collectAllSnapPoints(*tl, engine_.playback().currentTime(), true, activeClipId_);
            auto maxSnap = editor::AdvancedSnapEngine::getTimelineSnapThresholdInTicks(zoomFactor_, 8.0);
            auto snapRes = editor::AdvancedSnapEngine::resolveSortedTimelineSnap(targetTime, snapPoints, maxSnap);
            if (snapRes.hasSnapped) {
                targetTime = snapRes.snappedTime;
                snapIndicatorX_ = timeToPixel(targetTime);
            } else {
                snapIndicatorX_ = -1;
            }
        } else {
            snapIndicatorX_ = -1;
        }
        core::TimelineTime newDur = targetTime - dragStartTime_;
        if (newDur.ticks() > 0) {
            engine_.trimClipEnd(activeClipId_, newDur);
            update();
        }
    }
    QWidget::mouseMoveEvent(event);
}

void TimelineTracksWidget::mouseReleaseEvent(QMouseEvent* event) {
    snapIndicatorX_ = -1;
    if (dragMode_ != DragMode::None) {
        dragMode_ = DragMode::None;
        setCursor(Qt::ArrowCursor);
        update();
        event->accept();
        return;
    }
    update();
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

            editor::Clip clip(
                core::ClipId::generate(),
                (asset->type() == media::MediaType::Video) ? editor::ClipType::Video :
                (asset->type() == media::MediaType::Audio) ? editor::ClipType::Audio : editor::ClipType::Image,
                asset->fileName(),
                dropTime,
                asset->duration()
            );
            clip.setMediaId(asset->id());
            engine_.insertElement(std::move(clip), dropTime);

            dropTime = dropTime + asset->duration();
        }
    }

    event->acceptProposedAction();
    update();
}

} // namespace catchim::ui
#endif
