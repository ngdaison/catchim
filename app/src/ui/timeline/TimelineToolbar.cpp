#include "TimelineToolbar.h"

#if defined(HAVE_QT6)
#include <QHBoxLayout>

namespace catchim::ui {

TimelineToolbar::TimelineToolbar(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setFixedHeight(40);
    setupUi();
    refresh();
}

void TimelineToolbar::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);

    auto makeBtn = [this](const QString& text, const QString& tip) {
        auto* btn = new QPushButton(text, this);
        btn->setFixedSize(28, 28);
        btn->setToolTip(tip);
        btn->setStyleSheet(R"(
            QPushButton {
                background: transparent;
                color: #DEDEDE;
                font-size: 13px;
                border-radius: 4px;
            }
            QPushButton:hover {
                background: #242424;
            }
            QPushButton:checked {
                background: #00223D;
                color: #16A9F3;
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

    bookmarkBtn_ = makeBtn("🔖", "Đánh dấu (Bookmark/Marker)");
    connect(bookmarkBtn_, &QPushButton::clicked, [this]() {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            core::TimelineTime cur = engine_.playback().currentTime();
            tl->addBookmark({core::BookmarkId::generate(), cur, "Marker"});
        }
    });

    layout->addWidget(splitBtn_);
    layout->addWidget(dupBtn_);
    layout->addWidget(deleteBtn_);
    layout->addWidget(bookmarkBtn_);

    layout->addStretch();

    // Center Scene selector
    auto* sceneLabel = new QLabel("Cảnh chính (Main scene)", this);
    sceneLabel->setStyleSheet("color: #DEDEDE; font-weight: 500; font-size: 12px; padding: 2px 8px; background: #1A1A1A; border: 1px solid #292929; border-radius: 4px;");
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
    zoomSlider_->setStyleSheet(R"(
        QSlider::groove:horizontal {
            height: 4px;
            background: #292929;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: #16A9F3;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: #DEDEDE;
            width: 12px;
            margin-top: -4px;
            margin-bottom: -4px;
            border-radius: 6px;
        }
    )");
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
