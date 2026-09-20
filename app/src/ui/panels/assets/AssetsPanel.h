#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QStackedWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "editor/EditorEngine.h"
#include "media/MediaLibrary.h"

namespace catchim::ui {

class AssetsPanel : public QWidget {
    Q_OBJECT
public:
    AssetsPanel(editor::EditorEngine& engine, media::MediaLibrary& mediaLibrary, QWidget* parent = nullptr);

    void refresh();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void onImportClicked();
    void onMediaItemDoubleClicked(QListWidgetItem* item);
    void onAddMediaToTimeline(const core::MediaId& id);
    void onAddAudioSfx(const QString& name, double durationSec);
    void onAddTextPreset(const QString& title, const QString& fontStyle);
    void onAddGraphicPreset(const QString& name, const QString& shapeType);
    void onApplyEffectPreset(const QString& effectName);
    void onApplyTransitionPreset(const QString& name, double durationSec);
    void onApplyAdjustment(double brightness, double contrast, double saturation, double temperature, double tint);
    void onAddAdjustmentLayer();
    void onImportSrtClicked();
    void onAutoTranscribeClicked();
    void onCanvasAspectChanged(int width, int height);
    void onCanvasBgColorChanged(const QColor& color);

private:
    void setupUi();
    QWidget* createMediaView();
    QWidget* createAudioView();
    QWidget* createTextView();
    QWidget* createStickersView();
    QWidget* createEffectsView();
    QWidget* createTransitionsView();
    QWidget* createSubtitlesView();
    QWidget* createAdjustmentView();
    QWidget* createSettingsView();

    editor::EditorEngine& engine_;
    media::MediaLibrary& mediaLibrary_;

    QListWidget* tabList_{nullptr};
    QStackedWidget* viewsStack_{nullptr};
    QListWidget* mediaListWidget_{nullptr};
    QLineEdit* mediaSearchInput_{nullptr};
};

} // namespace catchim::ui
#endif
