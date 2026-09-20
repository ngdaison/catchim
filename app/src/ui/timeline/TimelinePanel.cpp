#include "TimelinePanel.h"

#if defined(HAVE_QT6)
#include <QVBoxLayout>
#include <QScrollBar>

namespace catchim::ui {

TimelinePanel::TimelinePanel(
    editor::EditorEngine& engine,
    media::MediaLibrary& mediaLibrary,
    QWidget* parent
)
    : QWidget(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    refresh();
}

void TimelinePanel::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 1. Toolbar
    toolbar_ = new TimelineToolbar(engine_, this);
    connect(toolbar_, &TimelineToolbar::zoomChanged, this, &TimelinePanel::onZoomChanged);
    mainLayout->addWidget(toolbar_);

    // 2. Ruler
    rulerWidget_ = new TimelineRulerWidget(engine_, this);
    connect(rulerWidget_, &TimelineRulerWidget::seekRequested, this, &TimelinePanel::onSeekRequested);
    mainLayout->addWidget(rulerWidget_);

    // 3. Tracks Area
    scrollArea_ = new QScrollArea(this);
    scrollArea_->setObjectName("timelineScrollArea");
    scrollArea_->setWidgetResizable(true);

    tracksWidget_ = new TimelineTracksWidget(engine_, mediaLibrary_, scrollArea_);
    connect(tracksWidget_, &TimelineTracksWidget::seekRequested, this, &TimelinePanel::onSeekRequested);
    connect(tracksWidget_, &TimelineTracksWidget::clipSelected, this, &TimelinePanel::clipSelected);

    scrollArea_->setWidget(tracksWidget_);
    mainLayout->addWidget(scrollArea_, 1);

    // Sync scrollbars
    connect(scrollArea_->horizontalScrollBar(), &QScrollBar::valueChanged, [this](int val) {
        rulerWidget_->setScrollOffset(val);
        tracksWidget_->setScrollOffset(val, scrollArea_->verticalScrollBar()->value());
    });
    connect(scrollArea_->verticalScrollBar(), &QScrollBar::valueChanged, [this](int val) {
        tracksWidget_->setScrollOffset(scrollArea_->horizontalScrollBar()->value(), val);
    });
}

void TimelinePanel::refresh() {
    toolbar_->refresh();
    rulerWidget_->update();
    tracksWidget_->update();
}

void TimelinePanel::onZoomChanged(double zoomFactor) {
    rulerWidget_->setZoomFactor(zoomFactor);
    tracksWidget_->setZoomFactor(zoomFactor);
}

void TimelinePanel::onSeekRequested(core::TimelineTime time) {
    engine_.seek(time);
    rulerWidget_->update();
    tracksWidget_->update();
}

} // namespace catchim::ui
#endif
