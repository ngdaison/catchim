#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QScrollArea>
#include "TimelineToolbar.h"
#include "TimelineRulerWidget.h"
#include "TimelineTracksWidget.h"
#include "editor/EditorEngine.h"
#include "media/MediaLibrary.h"

namespace catchim::ui {

class TimelinePanel : public QWidget {
    Q_OBJECT
public:
    TimelinePanel(
        editor::EditorEngine& engine,
        media::MediaLibrary& mediaLibrary,
        QWidget* parent = nullptr
    );

    void refresh();

signals:
    void clipSelected(core::ClipId clipId);

private slots:
    void onZoomChanged(double zoomFactor);
    void onSeekRequested(core::TimelineTime time);

private:
    void setupUi();

    editor::EditorEngine& engine_;
    media::MediaLibrary& mediaLibrary_;

    TimelineToolbar* toolbar_{nullptr};
    TimelineRulerWidget* rulerWidget_{nullptr};
    TimelineTracksWidget* tracksWidget_{nullptr};
    QScrollArea* scrollArea_{nullptr};
};

} // namespace catchim::ui
#endif
