#include "PropertiesPanel.h"

#if defined(HAVE_QT6)
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

namespace catchim::ui {

PropertiesPanel::PropertiesPanel(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setupUi();
    refresh();
}

void PropertiesPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // 1. Empty View
    emptyView_ = new QWidget(this);
    auto* evLayout = new QVBoxLayout(emptyView_);
    auto* evLabel = new QLabel("Chưa chọn phần tử nào\nHãy nhấp vào một clip trên timeline", emptyView_);
    evLabel->setAlignment(Qt::AlignCenter);
    evLabel->setStyleSheet("color: #808080; font-size: 12px;");
    evLayout->addStretch();
    evLayout->addWidget(evLabel);
    evLayout->addStretch();
    mainLayout->addWidget(emptyView_);

    // 2. Inspector View
    inspectorView_ = new QWidget(this);
    auto* insLayout = new QVBoxLayout(inspectorView_);
    insLayout->setContentsMargins(0, 0, 0, 0);

    auto* transformGroup = new QGroupBox("Biến đổi (Transform)", inspectorView_);
    transformGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #DEDEDE; border: 1px solid #292929; border-radius: 6px; margin-top: 8px; padding-top: 14px; }");
    auto* formLayout = new QFormLayout(transformGroup);
    formLayout->setSpacing(8);

    auto makeSpin = [this](double minVal, double maxVal, double step, double defaultVal) {
        auto* sb = new QDoubleSpinBox(this);
        sb->setRange(minVal, maxVal);
        sb->setSingleStep(step);
        sb->setValue(defaultVal);
        sb->setStyleSheet("background: #1A1A1A; color: #DEDEDE; border: 1px solid #292929; border-radius: 4px; padding: 2px 4px;");
        connect(sb, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::onTransformChanged);
        return sb;
    };

    posXSpin_ = makeSpin(-5000, 5000, 10, 0);
    posYSpin_ = makeSpin(-5000, 5000, 10, 0);
    scaleXSpin_ = makeSpin(0.01, 100, 0.05, 1.0);
    scaleYSpin_ = makeSpin(0.01, 100, 0.05, 1.0);
    rotateSpin_ = makeSpin(-360, 360, 1, 0);
    opacitySpin_ = makeSpin(0.0, 1.0, 0.05, 1.0);
    volumeSpin_ = makeSpin(0.0, 2.0, 0.05, 1.0);

    formLayout->addRow("Vị trí X:", posXSpin_);
    formLayout->addRow("Vị trí Y:", posYSpin_);
    formLayout->addRow("Tỷ lệ X:", scaleXSpin_);
    formLayout->addRow("Tỷ lệ Y:", scaleYSpin_);
    formLayout->addRow("Góc xoay (°):", rotateSpin_);
    formLayout->addRow("Độ mờ (Opacity):", opacitySpin_);
    formLayout->addRow("Âm lượng (Volume):", volumeSpin_);

    insLayout->addWidget(transformGroup);
    insLayout->addStretch();
    mainLayout->addWidget(inspectorView_);
}

void PropertiesPanel::refresh() {
    const auto& sel = engine_.selectedClips();
    if (sel.empty()) {
        emptyView_->setVisible(true);
        inspectorView_->setVisible(false);
        return;
    }

    emptyView_->setVisible(false);
    inspectorView_->setVisible(true);

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    const auto* clip = tl->findClip(sel[0]);
    if (!clip) return;

    isUpdatingUi_ = true;
    posXSpin_->setValue(clip->getParam<double>("transform.positionX", 0.0));
    posYSpin_->setValue(clip->getParam<double>("transform.positionY", 0.0));
    scaleXSpin_->setValue(clip->getParam<double>("transform.scaleX", 1.0));
    scaleYSpin_->setValue(clip->getParam<double>("transform.scaleY", 1.0));
    rotateSpin_->setValue(clip->getParam<double>("transform.rotate", 0.0));
    opacitySpin_->setValue(clip->getParam<double>("opacity", 1.0));
    volumeSpin_->setValue(clip->getParam<double>("volume", 1.0));
    isUpdatingUi_ = false;
}

void PropertiesPanel::onTransformChanged() {
    if (isUpdatingUi_) return;

    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    auto* clip = tl->findClip(sel[0]);
    if (!clip) return;

    clip->setParam("transform.positionX", posXSpin_->value());
    clip->setParam("transform.positionY", posYSpin_->value());
    clip->setParam("transform.scaleX", scaleXSpin_->value());
    clip->setParam("transform.scaleY", scaleYSpin_->value());
    clip->setParam("transform.rotate", rotateSpin_->value());
    clip->setParam("opacity", opacitySpin_->value());
    clip->setParam("volume", volumeSpin_->value());

    engine_.project().setDirty(true);
}

} // namespace catchim::ui
#endif
