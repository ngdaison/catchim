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
    setAttribute(Qt::WA_StyledBackground, true);
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
    toolbar->setStyleSheet("background-color: #0e0e11; border-top: 1px solid #27272a;");
    auto* tbLayout = new QHBoxLayout(toolbar);
    tbLayout->setContentsMargins(12, 4, 12, 4);
    tbLayout->setSpacing(8);

    // Timecode display
    timecodeLabel_ = new QLabel("00:00:00:00 / 00:00:00:00", toolbar);
    timecodeLabel_->setStyleSheet("font-family: 'Cascadia Code', monospace; color: #f4f4f5; font-size: 12px; font-weight: 600; padding: 4px 8px; background: #18181b; border-radius: 4px;");
    tbLayout->addWidget(timecodeLabel_);

    tbLayout->addStretch();

    auto makeTbBtn = [toolbar](const QString& text, const QString& tip) {
        auto* btn = new QPushButton(text, toolbar);
        btn->setFixedSize(32, 32);
        btn->setToolTip(tip);
        btn->setStyleSheet(R"(
            QPushButton {
                font-size: 13px;
                color: #f4f4f5;
                background-color: #18181b;
                border: 1px solid #27272a;
                border-radius: 6px;
                padding: 0;
            }
            QPushButton:hover {
                background-color: #27272a;
                border-color: #38bdf8;
                color: #38bdf8;
            }
        )");
        return btn;
    };

    auto* jumpStartBtn = makeTbBtn("⏮", "Về đầu video (Home)");
    connect(jumpStartBtn, &QPushButton::clicked, [this]() {
        engine_.seek(core::TimelineTime::zero());
        refresh();
    });
    tbLayout->addWidget(jumpStartBtn);

    auto* prevFrameBtn = makeTbBtn("◀", "Lùi 1 khung hình (Trái)");
    connect(prevFrameBtn, &QPushButton::clicked, [this]() {
        const auto& fps = engine_.project().settings().fps;
        core::TimelineTime cur = engine_.playback().currentTime();
        core::TimelineTime step = core::TimelineTime::fromSeconds(1.0 / (fps.denominator > 0 ? static_cast<double>(fps.numerator)/fps.denominator : 30.0));
        engine_.seek(cur > step ? cur - step : core::TimelineTime::zero());
        refresh();
    });
    tbLayout->addWidget(prevFrameBtn);

    // Play / Pause button
    playPauseBtn_ = new QPushButton("▶", toolbar);
    playPauseBtn_->setFixedSize(36, 32);
    playPauseBtn_->setToolTip("Phát / Tạm dừng (Space)");
    playPauseBtn_->setStyleSheet(R"(
        QPushButton {
            font-size: 14px;
            color: #ffffff;
            background-color: #0284c7;
            border: 1px solid #0284c7;
            border-radius: 6px;
            padding: 0;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #0369a1;
            border-color: #38bdf8;
        }
    )");
    connect(playPauseBtn_, &QPushButton::clicked, this, &PreviewPanel::onPlayPauseClicked);
    tbLayout->addWidget(playPauseBtn_);

    auto* nextFrameBtn = makeTbBtn("▶", "Tiến 1 khung hình (Phải)");
    connect(nextFrameBtn, &QPushButton::clicked, [this]() {
        const auto& fps = engine_.project().settings().fps;
        core::TimelineTime cur = engine_.playback().currentTime();
        core::TimelineTime step = core::TimelineTime::fromSeconds(1.0 / (fps.denominator > 0 ? static_cast<double>(fps.numerator)/fps.denominator : 30.0));
        engine_.seek(cur + step);
        refresh();
    });
    tbLayout->addWidget(nextFrameBtn);

    auto* jumpEndBtn = makeTbBtn("⏭", "Đến cuối video (End)");
    connect(jumpEndBtn, &QPushButton::clicked, [this]() {
        engine_.seek(engine_.project().totalDuration());
        refresh();
    });
    tbLayout->addWidget(jumpEndBtn);

    tbLayout->addStretch();

    // Zoom combo
    zoomCombo_ = new QComboBox(toolbar);
    zoomCombo_->addItems({"Fit", "25%", "50%", "75%", "100%", "150%", "200%"});
    zoomCombo_->setFixedWidth(84);
    zoomCombo_->setStyleSheet(R"(
        QComboBox {
            background-color: #18181b;
            color: #f4f4f5;
            border: 1px solid #27272a;
            border-radius: 6px;
            padding: 4px 8px;
            font-size: 11px;
            font-weight: 500;
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
