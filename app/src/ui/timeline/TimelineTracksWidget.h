#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include "editor/EditorEngine.h"
#include "media/MediaLibrary.h"

namespace catchim::ui {

class TimelineTracksWidget : public QWidget {
    Q_OBJECT
public:
    TimelineTracksWidget(
        editor::EditorEngine& engine,
        media::MediaLibrary& mediaLibrary,
        QWidget* parent = nullptr
    );

    void setZoomFactor(double zoom) { zoomFactor_ = zoom; update(); }
    void setScrollOffset(int x, int y) { scrollX_ = x; scrollY_ = y; update(); }

    int totalTracksHeight() const;

signals:
    void seekRequested(core::TimelineTime time);
    void clipSelected(core::ClipId clipId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    enum class DragMode { None, MovingClip, TrimmingStart, TrimmingEnd, BoxSelecting };

    struct HitTestResult {
        const editor::Track* track{nullptr};
        const editor::Clip* clip{nullptr};
        bool isTrimStart{false};
        bool isTrimEnd{false};
        bool isMuteBtn{false};
        bool isHideBtn{false};
        bool isLockBtn{false};
    };

    HitTestResult hitTest(const QPoint& pos) const;
    core::TimelineTime pixelToTime(int pixelX) const;
    int timeToPixel(core::TimelineTime time) const;

    editor::EditorEngine& engine_;
    media::MediaLibrary& mediaLibrary_;

    double zoomFactor_{1.0};
    int scrollX_{0};
    int scrollY_{0};

    DragMode dragMode_{DragMode::None};
    core::ClipId activeClipId_;
    QPoint dragStartPos_;
    core::TimelineTime dragStartTime_;
    core::TimelineTime dragStartDuration_;
    int snapIndicatorX_{-1};
};

} // namespace catchim::ui
#endif
