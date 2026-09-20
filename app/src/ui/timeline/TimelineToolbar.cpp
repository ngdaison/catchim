#include "TimelineToolbar.h"

#if defined(HAVE_QT6)
#include <QHBoxLayout>
#include <QMenu>

namespace catchim::ui {

TimelineToolbar::TimelineToolbar(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setFixedHeight(40);
    setStyleSheet("background-color: #0e0e11; border-bottom: 1px solid #27272a;");
    setupUi();
    refresh();
}

void TimelineToolbar::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 4, 10, 4);
    layout->setSpacing(6);

    auto makeBtn = [this](const QString& text, const QString& tip) {
        auto* btn = new QPushButton(text, this);
        btn->setFixedSize(30, 28);
        btn->setToolTip(tip);
        btn->setStyleSheet(R"(
            QPushButton {
                background: #18181b;
                color: #f4f4f5;
                font-size: 13px;
                border: 1px solid #27272a;
                border-radius: 6px;
                padding: 0;
            }
            QPushButton:hover {
                background: #27272a;
                border-color: #38bdf8;
                color: #38bdf8;
            }
            QPushButton:checked {
                background: #0284c7;
                border-color: #38bdf8;
                color: #ffffff;
            }
        )");
        return btn;
    };

    // Left tools
    splitBtn_ = makeBtn("✂", "Cắt phần tử tại con trỏ (S)");
    connect(splitBtn_, &QPushButton::clicked, this, &TimelineToolbar::onSplitClicked);

    dupBtn_ = makeBtn("❐", "Nhân bản phần tử (Ctrl+D)");
    connect(dupBtn_, &QPushButton::clicked, this, &TimelineToolbar::onDuplicateClicked);

    deleteBtn_ = makeBtn("🗑", "Xóa phần tử đang chọn (Delete)");
    connect(deleteBtn_, &QPushButton::clicked, this, &TimelineToolbar::onDeleteClicked);

    bookmarkBtn_ = makeBtn("🔖", "Đánh dấu (Bookmark/Marker - M)");
    connect(bookmarkBtn_, &QPushButton::clicked, [this]() {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            core::TimelineTime cur = engine_.playback().currentTime();
            tl->addBookmark({core::BookmarkId::generate(), cur, "Marker"});
        }
    });

    // Add Track Menu Button
    auto* addTrackBtn = new QPushButton("＋ Track", this);
    addTrackBtn->setFixedHeight(28);
    addTrackBtn->setStyleSheet(R"(
        QPushButton {
            background: #18181b;
            color: #f4f4f5;
            font-size: 11px;
            font-weight: 600;
            border: 1px solid #27272a;
            border-radius: 6px;
            padding: 0px 8px;
        }
        QPushButton:hover {
            background: #27272a;
            border-color: #38bdf8;
            color: #38bdf8;
        }
    )");

    auto* trackMenu = new QMenu(addTrackBtn);
    trackMenu->addAction("📹 Thêm Video Track", [this]() {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            tl->addTrack(editor::TrackType::Video, "Video " + std::to_string(tl->allTracks().size() + 1));
            engine_.project().setDirty(true);
        }
    });
    trackMenu->addAction("🎵 Thêm Audio Track", [this]() {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            tl->addTrack(editor::TrackType::Audio, "Audio " + std::to_string(tl->allTracks().size() + 1));
            engine_.project().setDirty(true);
        }
    });
    trackMenu->addAction("🆃 Thêm Text Track", [this]() {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            tl->addTrack(editor::TrackType::Text, "Text " + std::to_string(tl->allTracks().size() + 1));
            engine_.project().setDirty(true);
        }
    });
    addTrackBtn->setMenu(trackMenu);

    layout->addWidget(splitBtn_);
    layout->addWidget(dupBtn_);
    layout->addWidget(deleteBtn_);
    layout->addWidget(bookmarkBtn_);
    layout->addWidget(addTrackBtn);

    layout->addStretch();

    // Center Scene selector
    auto* sceneLabel = new QLabel("Cảnh chính (Main scene)", this);
    sceneLabel->setStyleSheet("color: #f4f4f5; font-weight: 600; font-size: 12px; padding: 4px 12px; background: #141417; border: 1px solid #27272a; border-radius: 6px;");
    layout->addWidget(sceneLabel);

    layout->addStretch();

    // Right tools
    snapBtn_ = makeBtn("🧲", "Tự động hít nam châm (N)");
    snapBtn_->setCheckable(true);
    snapBtn_->setChecked(engine_.isSnappingEnabled());
    connect(snapBtn_, &QPushButton::clicked, this, &TimelineToolbar::onSnappingToggled);

    rippleBtn_ = makeBtn("🌊", "Chế độ Ripple Editing");
    rippleBtn_->setCheckable(true);
    rippleBtn_->setChecked(engine_.isRippleEnabled());
    connect(rippleBtn_, &QPushButton::clicked, this, &TimelineToolbar::onRippleToggled);

    layout->addWidget(snapBtn_);
    layout->addWidget(rippleBtn_);

    // Zoom slider
    auto* zoomOutBtn = makeBtn("－", "Thu nhỏ");
    auto* zoomInBtn = makeBtn("＋", "Phóng to");

    zoomSlider_ = new QSlider(Qt::Horizontal, this);
    zoomSlider_->setFixedWidth(100);
    zoomSlider_->setRange(10, 500); // 10% to 500%
    zoomSlider_->setValue(100);
    connect(zoomSlider_, &QSlider::valueChanged, this, &TimelineToolbar::onZoomSliderChanged);

    connect(zoomOutBtn, &QPushButton::clicked, [this]() {
        zoomSlider_->setValue(std::max(10, zoomSlider_->value() - 25));
    });
    connect(zoomInBtn, &QPushButton::clicked, [this]() {
        zoomSlider_->setValue(std::min(500, zoomSlider_->value() + 25));
    });

    layout->addWidget(zoomOutBtn);
    layout->addWidget(zoomSlider_);
    layout->addWidget(zoomInBtn);
}

void TimelineToolbar::refresh() {
    snapBtn_->setChecked(engine_.isSnappingEnabled());
    rippleBtn_->setChecked(engine_.isRippleEnabled());
}

void TimelineToolbar::onSplitClicked() {
    engine_.splitAtPlayhead();
}

void TimelineToolbar::onDeleteClicked() {
    engine_.deleteSelectedClips();
}

void TimelineToolbar::onDuplicateClicked() {
    engine_.duplicateSelectedClips();
}

void TimelineToolbar::onSnappingToggled() {
    engine_.setSnappingEnabled(snapBtn_->isChecked());
}

void TimelineToolbar::onRippleToggled() {
    engine_.setRippleEnabled(rippleBtn_->isChecked());
}

void TimelineToolbar::onZoomSliderChanged(int val) {
    emit zoomChanged(static_cast<double>(val) / 100.0);
}

} // namespace catchim::ui
#endif
