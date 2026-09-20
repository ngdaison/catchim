#pragma once

#if defined(HAVE_QT6)
#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTabWidget>
#include "editor/EditorEngine.h"

namespace catchim::ui {

class PropertiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesPanel(editor::EditorEngine& engine, QWidget* parent = nullptr);

    void refresh();

private slots:
    void onTransformChanged();
    void onSpeedChanged();
    void onTextChanged();
    void onAudioChanged();
    void onGraphicChanged();

private:
    void setupUi();

    editor::EditorEngine& engine_;

    QWidget* emptyView_{nullptr};
    QWidget* inspectorView_{nullptr};

    QLabel* clipNameHeader_{nullptr};
    QLabel* clipTypeBadge_{nullptr};

    // Transform
    QDoubleSpinBox* posXSpin_{nullptr};
    QDoubleSpinBox* posYSpin_{nullptr};
    QDoubleSpinBox* scaleXSpin_{nullptr};
    QDoubleSpinBox* scaleYSpin_{nullptr};
    QDoubleSpinBox* rotateSpin_{nullptr};
    QSlider* opacitySlider_{nullptr};
    QLabel* opacityValLabel_{nullptr};
    QComboBox* blendModeCombo_{nullptr};

    // Speed / Retime
    QDoubleSpinBox* speedSpin_{nullptr};
    QSlider* speedSlider_{nullptr};
    QCheckBox* reverseCheck_{nullptr};

    // Text properties
    QWidget* textGroup_{nullptr};
    QTextEdit* textContentEdit_{nullptr};
    QDoubleSpinBox* fontSizeSpin_{nullptr};
    QPushButton* textColorBtn_{nullptr};
    std::string currentTextColor_{"#FFFFFF"};

    // Audio properties
    QWidget* audioGroup_{nullptr};
    QSlider* volumeSlider_{nullptr};
    QLabel* volumeValLabel_{nullptr};
    QDoubleSpinBox* fadeInSpin_{nullptr};
    QDoubleSpinBox* fadeOutSpin_{nullptr};

    // Graphic properties
    QWidget* graphicGroup_{nullptr};
    QComboBox* shapeCombo_{nullptr};
    QPushButton* graphicColorBtn_{nullptr};
    std::string currentGraphicColor_{"#38bdf8"};
    QDoubleSpinBox* cornerRadiusSpin_{nullptr};
    QDoubleSpinBox* strokeWidthSpin_{nullptr};

    // Active Effects & Transitions
    QWidget* activeEffectsGroup_{nullptr};
    QLabel* activeEffectLabel_{nullptr};
    QLabel* activeTransitionLabel_{nullptr};

    bool isUpdatingUi_{false};
};

} // namespace catchim::ui
#endif
