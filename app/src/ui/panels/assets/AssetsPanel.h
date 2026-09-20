#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include "editor/EditorEngine.h"
#include "media/MediaLibrary.h"

namespace catchim::ui {

class AssetsPanel : public QWidget {
    Q_OBJECT
public:
    AssetsPanel(editor::EditorEngine& engine, media::MediaLibrary& mediaLibrary, QWidget* parent = nullptr);

    void refresh();

private slots:
    void onImportClicked();
    void onMediaItemDoubleClicked(QListWidgetItem* item);

private:
    void setupUi();

    editor::EditorEngine& engine_;
    media::MediaLibrary& mediaLibrary_;

    QListWidget* tabList_{nullptr};
    QStackedWidget* viewsStack_{nullptr};
    QListWidget* mediaListWidget_{nullptr};
};

} // namespace catchim::ui
#endif
