#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "PreviewWidget.h"
#include "editor/EditorEngine.h"
#include "render/RenderEngine.h"
#include "media/MediaLibrary.h"

namespace catchim::ui {

class PreviewPanel : public QWidget {
    Q_OBJECT
public:
    PreviewPanel(
        editor::EditorEngine& engine,
        render::RenderEngine& renderEngine,
        media::MediaLibrary& mediaLibrary,
        QWidget* parent = nullptr
    );

    void refresh();
    void toggleSafeZones();
    void toggleFullscreen();

private slots:
    void onPlayPauseClicked();
    void onZoomChanged(int index);

private:
    void setupUi();

    editor::EditorEngine& engine_;
    PreviewWidget* previewWidget_{nullptr};
    QLabel* timecodeLabel_{nullptr};
    QPushButton* playPauseBtn_{nullptr};
    QComboBox* zoomCombo_{nullptr};
};

} // namespace catchim::ui
#endif
