#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QStackedWidget>
#include "editor/EditorEngine.h"

namespace catchim::ui {

class PropertiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesPanel(editor::EditorEngine& engine, QWidget* parent = nullptr);

    void refresh();

private slots:
    void onTransformChanged();

private:
    void setupUi();

    editor::EditorEngine& engine_;

    QWidget* emptyView_{nullptr};
    QWidget* inspectorView_{nullptr};

    QDoubleSpinBox* posXSpin_{nullptr};
    QDoubleSpinBox* posYSpin_{nullptr};
    QDoubleSpinBox* scaleXSpin_{nullptr};
    QDoubleSpinBox* scaleYSpin_{nullptr};
    QDoubleSpinBox* rotateSpin_{nullptr};
    QDoubleSpinBox* opacitySpin_{nullptr};
    QDoubleSpinBox* volumeSpin_{nullptr};

    bool isUpdatingUi_{false};
};

} // namespace catchim::ui
#endif
