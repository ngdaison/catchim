#include "PreviewPanel.h"

#if defined(HAVE_QT6)
#include "core/time/Timecode.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace catchim::ui {

PreviewPanel::PreviewPanel(
    editor::EditorEngine& engine,
    render::RenderEngine& renderEngine,
    media::MediaLibrary& mediaLibrary,
    QWidget* parent
)
    : QWidget(parent)
    , engine_(engine)
{
    previewWidget_ = new PreviewWidget(engine, renderEngine, mediaLibrary, this);
    setupUi();
    refresh();
}

void PreviewPanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(previewWidget_, 1);

    // Preview Toolbar
    auto* toolbar = new QWidget(this);
    toolbar->setFixedHeight(44);
    toolbar->setStyleSheet("background-color: #141414; border-top: 1px solid #292929;");
    auto* tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(12, 4, 12, 4);

    // Timecode display
    timecodeLabel_ = new QLabel("00:00:00:00 / 00:00:00:00", toolbar);
    timecodeLabel_->setStyleSheet("font-family: 'Cascadia Code', monospace; color: #DEDEDE; font-size: 12px;");
    tbLayout->addWidget(timecodeLabel_);

    tbLayout->addStretch();

    // Play / Pause button
    playPauseBtn_ = new QPushButton("▶", toolbar);
    playPauseBtn_->setFixedSize(36, 32);
    playPauseBtn_->setStyleSheet(R"(
        QPushButton {
            font-size: 14px;
            color: #DEDEDE;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #242424;
        }
    )");
    connect(playPauseBtn_, &QPushButton::clicked, this, &PreviewPanel::onPlayPauseClicked);
    tbLayout->addWidget(playPauseBtn_);

    tbLayout->addStretch();

    // Zoom combo
    zoomCombo_ = new QComboBox(toolbar);
    zoomCombo_->addItems({"Fit", "25%", "50%", "75%", "100%", "150%", "200%"});
    zoomCombo_->setFixedWidth(80);
    zoomCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: #1A1A1A;
            color: #DEDEDE;
            border: 1px solid #292929;
            border-radius: 4px;
            padding: 2px 6px;
            font-size: 11px;
        }
    )");
    connect(zoomCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PreviewPanel::onZoomChanged);
    tbLayout->addWidget(zoomCombo_);

    mainLayout->addWidget(toolbar);
}

void PreviewPanel::refresh() {
    core::TimelineTime curTime = engine_.playback().currentTime();
    core::TimelineTime dur = engine_.project().totalDuration();
    const auto& fps = engine_.project().settings().fps;

    std::string curTc = core::Timecode::format(curTime, core::TimecodeFormat::HH_MM_SS_FF, fps);
    std::string durTc = core::Timecode::format(dur, core::TimecodeFormat::HH_MM_SS_FF, fps);

    timecodeLabel_->setText(QString::fromStdString(curTc + " / " + durTc));
    playPauseBtn_->setText(engine_.playback().isPlaying() ? "⏸" : "▶");
    previewWidget_->update();
}

void PreviewPanel::onPlayPauseClicked() {
    engine_.togglePlay();
    refresh();
}

void PreviewPanel::onZoomChanged(int index) {
    if (index == 0) {
        previewWidget_->fitToScreen();
    } else {
        static const double factors[] = {-1.0, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0};
        previewWidget_->setZoomFactor(factors[index]);
    }
}

} // namespace catchim::ui
#endif
