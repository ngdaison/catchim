#include "MainWindow.h"

#if defined(HAVE_QT6)
#include "ui/theme/Theme.h"
#include "ui/export/ExportDialog.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QFileDialog>
#include <QProgressDialog>
#include <QProcess>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QFile>
#include <QPushButton>
#include <cmath>

namespace catchim::ui {

MainWindow::MainWindow(
    editor::EditorEngine& engine,
    media::MediaLibrary& mediaLibrary,
    render::RenderEngine& renderEngine,
    QWidget* parent
)
    : QMainWindow(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
    , renderEngine_(renderEngine)
{
    setWindowTitle("Catchim Video Editor");
    resize(1440, 900);

    setupUi();
    setupEngineCallbacks();

    // 60fps tick timer for playback and rendering
    tickTimer_ = new QTimer(this);
    connect(tickTimer_, &QTimer::timeout, this, &MainWindow::onAppTick);
    tickTimer_->start(16); // ~60 Hz
}

void MainWindow::setupUi() {
    auto* centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    centralWidget->setAttribute(Qt::WA_StyledBackground, true);
    setCentralWidget(centralWidget);

    auto* rootLayout = new QVBoxLayout(centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // 1. Top Header
    header_ = new EditorHeader(engine_, this);
    header_->setObjectName("editorHeader");
    connect(header_, &EditorHeader::exportClicked, this, &MainWindow::onExportRequested);
    connect(header_, &EditorHeader::themeToggleClicked, this, &MainWindow::onThemeToggleRequested);
    rootLayout->addWidget(header_);

    // 2. Main vertical splitter (Top: Panels, Bottom: Timeline)
    verticalSplitter_ = new QSplitter(Qt::Vertical, centralWidget);
    verticalSplitter_->setObjectName("verticalSplitter");
    verticalSplitter_->setHandleWidth(3);

    // 3. Top horizontal splitter (Assets, Preview, Properties)
    horizontalSplitter_ = new QSplitter(Qt::Horizontal, verticalSplitter_);
    horizontalSplitter_->setObjectName("horizontalSplitter");
    horizontalSplitter_->setHandleWidth(3);

    assetsPanel_ = new AssetsPanel(engine_, mediaLibrary_, horizontalSplitter_);
    assetsPanel_->setObjectName("assetsPanel");
    assetsPanel_->setProperty("class", "Panel");

    previewPanel_ = new PreviewPanel(engine_, renderEngine_, mediaLibrary_, horizontalSplitter_);
    previewPanel_->setObjectName("previewPanel");
    previewPanel_->setProperty("class", "Panel");

    propertiesPanel_ = new PropertiesPanel(engine_, horizontalSplitter_);
    propertiesPanel_->setObjectName("propertiesPanel");
    propertiesPanel_->setProperty("class", "Panel");

    horizontalSplitter_->addWidget(assetsPanel_);
    horizontalSplitter_->addWidget(previewPanel_);
    horizontalSplitter_->addWidget(propertiesPanel_);

    // Set horizontal proportions (20% Assets, 55% Preview, 25% Properties)
    horizontalSplitter_->setStretchFactor(0, 20);
    horizontalSplitter_->setStretchFactor(1, 55);
    horizontalSplitter_->setStretchFactor(2, 25);

    verticalSplitter_->addWidget(horizontalSplitter_);

    // 4. Bottom Timeline
    timelinePanel_ = new TimelinePanel(engine_, mediaLibrary_, verticalSplitter_);
    timelinePanel_->setObjectName("timelinePanel");
    timelinePanel_->setProperty("class", "Panel");
    connect(timelinePanel_, &TimelinePanel::clipSelected, [this](core::ClipId /* id */) {
        propertiesPanel_->refresh();
    });

    verticalSplitter_->addWidget(timelinePanel_);

    // Proportions (60% Panels, 40% Timeline)
    verticalSplitter_->setStretchFactor(0, 60);
    verticalSplitter_->setStretchFactor(1, 40);

    rootLayout->addWidget(verticalSplitter_, 1);
}

void MainWindow::setupEngineCallbacks() {
    engine_.setOnProjectChanged([this]() {
        header_->refresh();
        previewPanel_->refresh();
        timelinePanel_->refresh();
        propertiesPanel_->refresh();
    });

    engine_.setOnTimelineChanged([this]() {
        previewPanel_->refresh();
        timelinePanel_->refresh();
    });

    engine_.setOnSelectionChanged([this]() {
        propertiesPanel_->refresh();
        timelinePanel_->refresh();
    });
}

void MainWindow::onAppTick() {
    if (engine_.playback().isPlaying()) {
        engine_.update();
        previewPanel_->refresh();
        timelinePanel_->refresh();
    }
}

void MainWindow::onExportRequested() {
    ExportDialog dlg(engine_, mediaLibrary_, renderEngine_, this);
    dlg.exec();
}

void MainWindow::onThemeToggleRequested() {
    auto currentMode = Theme::instance().mode();
    Theme::instance().setTheme(currentMode == ThemeMode::Dark ? ThemeMode::Light : ThemeMode::Dark);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    // 1. Playback & Navigation
    if (event->key() == Qt::Key_Space) {
        engine_.togglePlay();
        previewPanel_->refresh();
        event->accept();
        return;
    }

    if (event->modifiers() == Qt::NoModifier) {
        if (event->key() == Qt::Key_J) {
            engine_.playback().setPlaybackRate(-1.5);
            if (!engine_.playback().isPlaying()) engine_.play();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_K) {
            engine_.pause();
            event->accept();
            return;
        }
        if (event->key() == Qt::Key_L) {
            engine_.playback().setPlaybackRate(1.5);
            if (!engine_.playback().isPlaying()) engine_.play();
            event->accept();
            return;
        }
    }

    if (event->key() == Qt::Key_Left) {
        const auto& fps = engine_.project().settings().fps;
        engine_.playback().stepFrame(-1, fps);
        previewPanel_->refresh();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Right) {
        const auto& fps = engine_.project().settings().fps;
        engine_.playback().stepFrame(1, fps);
        previewPanel_->refresh();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Home) {
        engine_.seek(core::TimelineTime::zero());
        previewPanel_->refresh();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_End) {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            engine_.seek(tl->totalDuration());
            previewPanel_->refresh();
            timelinePanel_->refresh();
        }
        event->accept();
        return;
    }

    // 2. Timeline Editing
    if (event->key() == Qt::Key_S && event->modifiers() == Qt::NoModifier) {
        engine_.splitAtPlayhead();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Q && event->modifiers() == Qt::NoModifier) {
        engine_.splitLeftAtPlayhead();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_W && event->modifiers() == Qt::NoModifier) {
        engine_.splitRightAtPlayhead();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        engine_.deleteSelectedClips();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_R && event->modifiers() == Qt::NoModifier) {
        engine_.toggleRipple();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_N && event->modifiers() == Qt::NoModifier) {
        engine_.toggleSnapping();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_M && event->modifiers() == Qt::NoModifier) {
        engine_.toggleBookmarkAtPlayhead();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    // 3. History & Selection
    if (event->matches(QKeySequence::Undo)) {
        engine_.undo();
        event->accept();
        return;
    }

    if (event->matches(QKeySequence::Redo)) {
        engine_.redo();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_D && (event->modifiers() & Qt::ControlModifier)) {
        engine_.duplicateSelectedClips();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_A && (event->modifiers() & Qt::ControlModifier)) {
        auto* tl = engine_.activeTimeline();
        if (tl) {
            std::vector<core::ClipId> allIds;
            for (const auto* track : tl->allTracks()) {
                for (const auto& c : track->clips()) {
                    allIds.push_back(c.id());
                }
            }
            engine_.selectClips(allIds, false);
        }
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        engine_.deselectAll();
        event->accept();
        return;
    }

    // 4. Preview & Export
    if (event->key() == Qt::Key_E && (event->modifiers() & Qt::ControlModifier)) {
        onExportRequested();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_F11 || (event->key() == Qt::Key_F && event->modifiers() == Qt::NoModifier)) {
        previewPanel_->toggleFullscreen();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Z && event->modifiers() == Qt::NoModifier) {
        previewPanel_->toggleSafeZones();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

} // namespace catchim::ui
#endif
