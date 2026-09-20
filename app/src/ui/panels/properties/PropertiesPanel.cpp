#include "PropertiesPanel.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include "ui/theme/Theme.h"
#include "core/time/Timecode.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <QScrollArea>

namespace catchim::ui {

PropertiesPanel::PropertiesPanel(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    refresh();
}

void PropertiesPanel::setupUi() {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Empty View
    emptyView_ = new QWidget(this);
    auto* evLayout = new QVBoxLayout(emptyView_);
    emptyIcon_ = new QLabel(emptyView_);
    emptyIcon_->setPixmap(UiIcons::getPixmap(UiIcon::Target, Theme::instance().palette().textSecondary, 36));
    emptyIcon_->setAlignment(Qt::AlignCenter);
    emptyLabel_ = new QLabel("Chưa chọn phần tử nào\nHãy nhấp vào một clip trên timeline để xem và chỉnh sửa thuộc tính", emptyView_);
    emptyLabel_->setAlignment(Qt::AlignCenter);
    emptyLabel_->setProperty("class", "SecondaryLabel");
    evLayout->addStretch();
    evLayout->addWidget(emptyIcon_);
    evLayout->addSpacing(8);
    evLayout->addWidget(emptyLabel_);
    evLayout->addStretch();
    rootLayout->addWidget(emptyView_);

    // 2. Inspector Scroll Area
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    inspectorView_ = new QWidget(scrollArea);
    auto* insLayout = new QVBoxLayout(inspectorView_);
    insLayout->setContentsMargins(12, 12, 12, 12);
    insLayout->setSpacing(12);

    // Header info card
    auto* headerCard = new QWidget(inspectorView_);
    headerCard->setProperty("class", "cardWidget");
    auto* hLayout = new QHBoxLayout(headerCard);
    hLayout->setContentsMargins(8, 8, 8, 8);
    clipTypeBadge_ = new QLabel("VIDEO", headerCard);
    clipTypeBadge_->setStyleSheet("background: #0284c7; color: #ffffff; font-weight: 700; font-size: 10px; padding: 2px 6px; border-radius: 4px;");
    clipNameHeader_ = new QLabel("Clip Name", headerCard);
    clipNameHeader_->setProperty("class", "SectionTitle");
    hLayout->addWidget(clipTypeBadge_);
    hLayout->addWidget(clipNameHeader_, 1);
    insLayout->addWidget(headerCard);

    auto makeSpin = [this](double minVal, double maxVal, double step, double defaultVal) {
        auto* sb = new QDoubleSpinBox(this);
        sb->setRange(minVal, maxVal);
        sb->setSingleStep(step);
        sb->setValue(defaultVal);
        return sb;
    };

    // A. Transform Group
    auto* transformGroup = new QGroupBox("Biến đổi vị trí (Transform)", inspectorView_);
    auto* tForm = new QFormLayout(transformGroup);
    tForm->setSpacing(8);

    posXSpin_ = makeSpin(-5000, 5000, 10, 0);
    connect(posXSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onTransformChanged);
    posYSpin_ = makeSpin(-5000, 5000, 10, 0);
    connect(posYSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onTransformChanged);
    scaleXSpin_ = makeSpin(0.01, 100, 0.05, 1.0);
    connect(scaleXSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onTransformChanged);
    scaleYSpin_ = makeSpin(0.01, 100, 0.05, 1.0);
    connect(scaleYSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onTransformChanged);
    rotateSpin_ = makeSpin(-360, 360, 1, 0);
    connect(rotateSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onTransformChanged);

    opacitySlider_ = new QSlider(Qt::Horizontal, transformGroup);
    opacitySlider_->setRange(0, 100);
    opacitySlider_->setValue(100);
    opacityValLabel_ = new QLabel("100%", transformGroup);
    opacityValLabel_->setFixedWidth(36);
    auto* opLayout = new QHBoxLayout();
    opLayout->addWidget(opacitySlider_, 1);
    opLayout->addWidget(opacityValLabel_);
    connect(opacitySlider_, &QSlider::valueChanged, [this](int v) {
        opacityValLabel_->setText(QString("%1%").arg(v));
        onTransformChanged();
    });

    blendModeCombo_ = new QComboBox(transformGroup);
    blendModeCombo_->addItems({"Bình thường (Normal)", "Nhân màu (Multiply)", "Làm sáng (Screen)", "Phủ lên (Overlay)", "Tối màu (Darken)", "Sáng màu (Lighten)", "Né màu (Color Dodge)"});
    connect(blendModeCombo_, &QComboBox::currentIndexChanged, this, &PropertiesPanel::onTransformChanged);

    tForm->addRow("Vị trí X (px):", posXSpin_);
    tForm->addRow("Vị trí Y (px):", posYSpin_);
    tForm->addRow("Tỷ lệ X:", scaleXSpin_);
    tForm->addRow("Tỷ lệ Y:", scaleYSpin_);
    tForm->addRow("Góc xoay (°):", rotateSpin_);
    tForm->addRow("Độ mờ (Opacity):", opLayout);
    tForm->addRow("Hòa trộn (Blend):", blendModeCombo_);
    insLayout->addWidget(transformGroup);

    // B. Speed / Retime Group
    auto* speedGroup = new QGroupBox("Tốc độ phát (Speed / Retime)", inspectorView_);
    auto* sForm = new QFormLayout(speedGroup);
    sForm->setSpacing(8);

    speedSpin_ = makeSpin(0.1, 10.0, 0.1, 1.0);
    connect(speedSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onSpeedChanged);
    speedSlider_ = new QSlider(Qt::Horizontal, speedGroup);
    speedSlider_->setRange(10, 500); // 0.1x to 5.0x
    speedSlider_->setValue(100);
    connect(speedSlider_, &QSlider::valueChanged, [this](int v) {
        if (!isUpdatingUi_) {
            speedSpin_->setValue(static_cast<double>(v) / 100.0);
        }
    });

    auto* spdLayout = new QHBoxLayout();
    spdLayout->addWidget(speedSlider_, 1);
    spdLayout->addWidget(speedSpin_);

    reverseCheck_ = new QCheckBox("Phát đảo ngược chiều", speedGroup);
    connect(reverseCheck_, &QCheckBox::toggled, this, &PropertiesPanel::onSpeedChanged);

    sForm->addRow("Tốc độ:", spdLayout);
    sForm->addRow("", reverseCheck_);
    insLayout->addWidget(speedGroup);

    // C. Text Formatting Group
    textGroup_ = new QGroupBox("Nội dung & Định dạng chữ", inspectorView_);
    auto* txtForm = new QFormLayout(textGroup_);
    txtForm->setSpacing(8);

    textContentEdit_ = new QTextEdit(textGroup_);
    textContentEdit_->setFixedHeight(64);
    textContentEdit_->setPlaceholderText("Nhập nội dung văn bản ở đây...");
    connect(textContentEdit_, &QTextEdit::textChanged, this, &PropertiesPanel::onTextChanged);

    fontSizeSpin_ = makeSpin(6, 300, 2, 48);
    connect(fontSizeSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onTextChanged);

    textColorBtn_ = new QPushButton("Chọn màu chữ...", textGroup_);
    textColorBtn_->setFixedHeight(28);
    connect(textColorBtn_, &QPushButton::clicked, [this]() {
        QColor col = QColorDialog::getColor(Qt::white, this, "Chọn màu chữ");
        if (col.isValid()) {
            currentTextColor_ = col.name().toStdString();
            textColorBtn_->setStyleSheet(QString("background: %1; color: %2; font-weight: bold; border-radius: 4px;")
                .arg(col.name())
                .arg((col.lightness() > 128) ? "#000000" : "#FFFFFF"));
            onTextChanged();
        }
    });

    txtForm->addRow("Nội dung:", textContentEdit_);
    txtForm->addRow("Cỡ chữ (px):", fontSizeSpin_);
    txtForm->addRow("Màu chữ:", textColorBtn_);
    insLayout->addWidget(textGroup_);

    // D. Audio Controls Group
    audioGroup_ = new QGroupBox("Âm thanh (Audio)", inspectorView_);
    auto* aForm = new QFormLayout(audioGroup_);
    aForm->setSpacing(8);

    volumeSlider_ = new QSlider(Qt::Horizontal, audioGroup_);
    volumeSlider_->setRange(0, 200);
    volumeSlider_->setValue(100);
    volumeValLabel_ = new QLabel("100%", audioGroup_);
    volumeValLabel_->setFixedWidth(36);
    auto* volLayout = new QHBoxLayout();
    volLayout->addWidget(volumeSlider_, 1);
    volLayout->addWidget(volumeValLabel_);
    connect(volumeSlider_, &QSlider::valueChanged, [this](int v) {
        volumeValLabel_->setText(QString("%1%").arg(v));
        onAudioChanged();
    });

    fadeInSpin_ = makeSpin(0.0, 10.0, 0.1, 0.0);
    connect(fadeInSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onAudioChanged);
    fadeOutSpin_ = makeSpin(0.0, 10.0, 0.1, 0.0);
    connect(fadeOutSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onAudioChanged);

    aForm->addRow("Âm lượng:", volLayout);
    aForm->addRow("Mờ dần vào (s):", fadeInSpin_);
    aForm->addRow("Mờ dần ra (s):", fadeOutSpin_);
    insLayout->addWidget(audioGroup_);

    // E. Graphic Controls Group
    graphicGroup_ = new QGroupBox("Hình khối (Graphic / Shape)", inspectorView_);
    auto* gForm = new QFormLayout(graphicGroup_);
    gForm->setSpacing(8);

    shapeCombo_ = new QComboBox(graphicGroup_);
    shapeCombo_->addItem("Hình chữ nhật (Rectangle)", "rectangle");
    shapeCombo_->addItem("Hình tròn (Circle)", "circle");
    shapeCombo_->addItem("Ngôi sao (Star)", "star");
    shapeCombo_->addItem("Mũi tên (Arrow)", "arrow");
    shapeCombo_->addItem("Huy hiệu (Badge)", "badge");
    connect(shapeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertiesPanel::onGraphicChanged);

    graphicColorBtn_ = new QPushButton("Chọn màu", graphicGroup_);
    graphicColorBtn_->setFixedHeight(28);
    connect(graphicColorBtn_, &QPushButton::clicked, [this]() {
        QColor col = QColorDialog::getColor(QColor(QString::fromStdString(currentGraphicColor_)), this, "Chọn màu hình khối");
        if (col.isValid()) {
            currentGraphicColor_ = col.name().toStdString();
            graphicColorBtn_->setStyleSheet(QString("background: %1; color: %2; font-weight: bold; border-radius: 4px;")
                .arg(col.name()).arg(col.lightness() > 128 ? "#000000" : "#FFFFFF"));
            onGraphicChanged();
        }
    });

    cornerRadiusSpin_ = makeSpin(0.0, 100.0, 1.0, 8.0);
    connect(cornerRadiusSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onGraphicChanged);

    strokeWidthSpin_ = makeSpin(0.0, 50.0, 1.0, 0.0);
    connect(strokeWidthSpin_, &QDoubleSpinBox::valueChanged, this, &PropertiesPanel::onGraphicChanged);

    gForm->addRow("Loại hình:", shapeCombo_);
    gForm->addRow("Màu tô:", graphicColorBtn_);
    gForm->addRow("Bo góc (px):", cornerRadiusSpin_);
    gForm->addRow("Độ dày viền:", strokeWidthSpin_);
    insLayout->addWidget(graphicGroup_);

    // F. Active Effects & Transitions
    activeEffectsGroup_ = new QGroupBox("Hiệu ứng & Chuyển cảnh đang gắn", inspectorView_);
    auto* effLayout = new QVBoxLayout(activeEffectsGroup_);
    effLayout->setSpacing(6);
    activeEffectLabel_ = new QLabel("Hiệu ứng: Không có", activeEffectsGroup_);
    activeEffectLabel_->setProperty("class", "SecondaryLabel");
    activeTransitionLabel_ = new QLabel("Chuyển cảnh: Không có", activeEffectsGroup_);
    activeTransitionLabel_->setProperty("class", "SecondaryLabel");
    effLayout->addWidget(activeEffectLabel_);
    effLayout->addWidget(activeTransitionLabel_);
    insLayout->addWidget(activeEffectsGroup_);

    insLayout->addStretch();
    scrollArea->setWidget(inspectorView_);
    rootLayout->addWidget(scrollArea, 1);
}

void PropertiesPanel::refresh() {
    if (emptyIcon_) {
        emptyIcon_->setPixmap(UiIcons::getPixmap(UiIcon::Target, Theme::instance().palette().textSecondary, 36));
    }

    const auto& sel = engine_.selectedClips();
    if (sel.empty()) {
        emptyView_->setVisible(true);
        inspectorView_->setVisible(false);
        return;
    }

    auto* tl = engine_.activeTimeline();
    if (!tl) {
        emptyView_->setVisible(true);
        inspectorView_->setVisible(false);
        return;
    }

    const auto* clip = tl->findClip(sel[0]);
    if (!clip) {
        emptyView_->setVisible(true);
        inspectorView_->setVisible(false);
        return;
    }

    emptyView_->setVisible(false);
    inspectorView_->setVisible(true);

    isUpdatingUi_ = true;

    // Header info
    clipNameHeader_->setText(QString::fromStdString(clip->name()));
    QString typeStr = "VIDEO";
    if (clip->type() == editor::ClipType::Audio) typeStr = "AUDIO";
    else if (clip->type() == editor::ClipType::Text) typeStr = "TEXT";
    else if (clip->type() == editor::ClipType::Graphic) typeStr = "GRAPHIC";
    else if (clip->type() == editor::ClipType::Image) typeStr = "IMAGE";
    clipTypeBadge_->setText(typeStr);

    // Transform
    posXSpin_->setValue(clip->getParam<double>("transform.positionX", 0.0));
    posYSpin_->setValue(clip->getParam<double>("transform.positionY", 0.0));
    scaleXSpin_->setValue(clip->getParam<double>("transform.scaleX", 1.0));
    scaleYSpin_->setValue(clip->getParam<double>("transform.scaleY", 1.0));
    rotateSpin_->setValue(clip->getParam<double>("transform.rotate", 0.0));

    double op = clip->getParam<double>("opacity", 1.0);
    opacitySlider_->setValue(static_cast<int>(op * 100));
    opacityValLabel_->setText(QString("%1%").arg(static_cast<int>(op * 100)));

    int blendIdx = clip->getParam<int>("transform.blendMode", 0);
    blendModeCombo_->setCurrentIndex(std::clamp(blendIdx, 0, blendModeCombo_->count() - 1));

    // Speed
    double speed = clip->getParam<double>("speed", 1.0);
    if (speed <= 0.01) speed = 1.0;
    speedSpin_->setValue(speed);
    speedSlider_->setValue(static_cast<int>(speed * 100));
    reverseCheck_->setChecked(clip->getParam<bool>("reversed", false));

    // Text group visibility & values
    bool isText = (clip->type() == editor::ClipType::Text);
    textGroup_->setVisible(isText);
    if (isText) {
        textContentEdit_->setPlainText(QString::fromStdString(clip->getParam<std::string>("text.content", clip->name())));
        fontSizeSpin_->setValue(clip->getParam<double>("text.fontSize", 48.0));
        currentTextColor_ = clip->getParam<std::string>("text.color", "#FFFFFF");
        textColorBtn_->setStyleSheet(QString("background: %1; color: %2; font-weight: bold; border-radius: 4px;")
            .arg(QString::fromStdString(currentTextColor_))
            .arg("#000000"));
    }

    // Audio group visibility & values
    bool hasAudio = (clip->type() == editor::ClipType::Audio || clip->type() == editor::ClipType::Video);
    audioGroup_->setVisible(hasAudio);
    if (hasAudio) {
        double vol = clip->getParam<double>("volume", 1.0);
        volumeSlider_->setValue(static_cast<int>(vol * 100));
        volumeValLabel_->setText(QString("%1%").arg(static_cast<int>(vol * 100)));
        fadeInSpin_->setValue(clip->getParam<double>("audio.fadeIn", 0.0));
        fadeOutSpin_->setValue(clip->getParam<double>("audio.fadeOut", 0.0));
    }

    // Graphic group visibility & values
    bool isGraphic = (clip->type() == editor::ClipType::Graphic);
    graphicGroup_->setVisible(isGraphic);
    if (isGraphic) {
        std::string shape = clip->getParam<std::string>("graphic.shape", "rectangle");
        int shapeIdx = shapeCombo_->findData(QString::fromStdString(shape));
        if (shapeIdx >= 0) shapeCombo_->setCurrentIndex(shapeIdx);

        currentGraphicColor_ = clip->getParam<std::string>("graphic.color", "#38bdf8");
        QColor col(QString::fromStdString(currentGraphicColor_));
        graphicColorBtn_->setStyleSheet(QString("background: %1; color: %2; font-weight: bold; border-radius: 4px;")
            .arg(col.name()).arg(col.lightness() > 128 ? "#000000" : "#FFFFFF"));

        cornerRadiusSpin_->setValue(clip->getParam<double>("graphic.cornerRadius", 8.0));
        strokeWidthSpin_->setValue(clip->getParam<double>("graphic.strokeWidth", 0.0));
    }

    // Active Effects / Transitions
    std::string effName = clip->getParam<std::string>("effect.name", "");
    if (!effName.empty()) {
        activeEffectLabel_->setText(QString("Hiệu ứng: <b>%1</b>").arg(QString::fromStdString(effName)));
    } else {
        activeEffectLabel_->setText("Hiệu ứng: Không có");
    }

    std::string transName = clip->getParam<std::string>("transition.type", "");
    if (!transName.empty()) {
        double transDur = clip->getParam<double>("transition.duration", 0.5);
        activeTransitionLabel_->setText(QString("Chuyển cảnh: <b>%1</b> (%2s)").arg(QString::fromStdString(transName)).arg(transDur, 0, 'f', 1));
    } else {
        activeTransitionLabel_->setText("Chuyển cảnh: Không có");
    }

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
    clip->setParam("opacity", static_cast<double>(opacitySlider_->value()) / 100.0);
    clip->setParam("transform.blendMode", blendModeCombo_->currentIndex());

    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void PropertiesPanel::onSpeedChanged() {
    if (isUpdatingUi_) return;

    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;
    auto* tl = engine_.activeTimeline();
    if (!tl) return;
    auto* clip = tl->findClip(sel[0]);
    if (!clip) return;

    clip->setParam("speed", speedSpin_->value());
    clip->setParam("reversed", reverseCheck_->isChecked());

    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void PropertiesPanel::onTextChanged() {
    if (isUpdatingUi_) return;

    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;
    auto* tl = engine_.activeTimeline();
    if (!tl) return;
    auto* clip = tl->findClip(sel[0]);
    if (!clip) return;

    clip->setName(textContentEdit_->toPlainText().toStdString());
    clip->setParam("text.content", textContentEdit_->toPlainText().toStdString());
    clip->setParam("text.fontSize", fontSizeSpin_->value());
    clip->setParam("text.color", currentTextColor_);

    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void PropertiesPanel::onAudioChanged() {
    if (isUpdatingUi_) return;

    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;
    auto* tl = engine_.activeTimeline();
    if (!tl) return;
    auto* clip = tl->findClip(sel[0]);
    if (!clip) return;

    clip->setParam("volume", static_cast<double>(volumeSlider_->value()) / 100.0);
    clip->setParam("audio.fadeIn", fadeInSpin_->value());
    clip->setParam("audio.fadeOut", fadeOutSpin_->value());

    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void PropertiesPanel::onGraphicChanged() {
    if (isUpdatingUi_) return;

    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;
    auto* tl = engine_.activeTimeline();
    if (!tl) return;
    auto* clip = tl->findClip(sel[0]);
    if (!clip) return;

    clip->setParam("graphic.shape", shapeCombo_->currentData().toString().toStdString());
    clip->setParam("graphic.color", currentGraphicColor_);
    clip->setParam("graphic.cornerRadius", cornerRadiusSpin_->value());
    clip->setParam("graphic.strokeWidth", strokeWidthSpin_->value());

    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

} // namespace catchim::ui
#endif
