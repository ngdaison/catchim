#include "MainWindow.h"

#if defined(HAVE_QT6)
#include "ui/theme/Theme.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QMessageBox>
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
    auto* tl = engine_.activeTimeline();
    if (!tl || tl->mainTrack().clips().empty()) {
        QMessageBox::warning(this, "Xuất video", "Dự án đang trống. Vui lòng thêm video, ảnh hoặc âm thanh vào timeline trước khi xuất!");
        return;
    }

    core::TimelineTime totalDur = tl->totalDuration();
    double totalSec = totalDur.toSeconds();
    if (totalSec <= 0.0) {
        QMessageBox::warning(this, "Xuất video", "Thời lượng dự án không hợp lệ.");
        return;
    }

    QString defaultName = QString::fromStdString(engine_.project().name()) + ".mp4";
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Xuất video MP4",
        defaultName,
        "Video MP4 (*.mp4);;All Files (*.*)"
    );
    if (savePath.isEmpty()) return;

    int width = engine_.project().settings().canvasSize.width;
    int height = engine_.project().settings().canvasSize.height;
    if (width <= 0 || height <= 0) {
        width = 1920;
        height = 1080;
    }

    const auto& fpsSetting = engine_.project().settings().fps;
    double fps = (fpsSetting.denominator > 0) ?
        static_cast<double>(fpsSetting.numerator) / fpsSetting.denominator : 30.0;
    if (fps <= 0.0) fps = 30.0;

    int totalFrames = static_cast<int>(std::ceil(totalSec * fps));
    if (totalFrames <= 0) totalFrames = 1;

    // Locate FFmpeg
    QString ffmpegPath = "ffmpeg";
    static const QString knownWingetFfmpeg = "C:/Users/sonng/AppData/Local/Microsoft/WinGet/Packages/Gyan.FFmpeg_Microsoft.Winget.Source_8wekyb3d8bbwe/ffmpeg-9.0.1-full_build/bin/ffmpeg.exe";
    if (QFile::exists(knownWingetFfmpeg)) {
        ffmpegPath = knownWingetFfmpeg;
    }

    QStringList args;
    args << "-y"
         << "-f" << "rawvideo"
         << "-vcodec" << "rawvideo"
         << "-s" << QString("%1x%2").arg(width).arg(height)
         << "-pix_fmt" << "rgba"
         << "-r" << QString::number(fps, 'f', 2)
         << "-i" << "-"
         << "-c:v" << "libx264"
         << "-pix_fmt" << "yuv420p"
         << "-preset" << "fast"
         << "-crf" << "22"
         << savePath;

    QProcess ffmpegProcess;
    ffmpegProcess.start(ffmpegPath, args);
    if (!ffmpegProcess.waitForStarted(5000)) {
        QMessageBox::critical(this, "Lỗi xuất video",
            "Không thể khởi chạy FFmpeg để kết xuất video.\n"
            "Vui lòng kiểm tra FFmpeg đã được cài đặt trong hệ thống.");
        return;
    }

    QProgressDialog progress("Đang kết xuất video MP4...", "Hủy bỏ", 0, totalFrames, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);
    progress.setStyleSheet(R"(
        QProgressDialog {
            background-color: #18181b;
            color: #f4f4f5;
            border: 1px solid #27272a;
        }
        QLabel {
            color: #f4f4f5;
            font-size: 13px;
        }
        QProgressBar {
            background-color: #27272a;
            color: #f4f4f5;
            border: 1px solid #3f3f46;
            border-radius: 4px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #0284c7;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #27272a;
            color: #f4f4f5;
            border: 1px solid #3f3f46;
            padding: 4px 12px;
            border-radius: 4px;
        }
    )");

    renderEngine_.setCanvasSize(width, height);

    bool canceled = false;
    for (int f = 0; f < totalFrames; ++f) {
        if (progress.wasCanceled()) {
            canceled = true;
            ffmpegProcess.kill();
            break;
        }

        double timeSec = static_cast<double>(f) / fps;
        core::TimelineTime curTime = core::TimelineTime::fromSeconds(timeSec);
        const auto& frame = renderEngine_.renderFrame(engine_.project(), mediaLibrary_, curTime);

        const char* pixelData = reinterpret_cast<const char*>(frame.rgbaPixels.data());
        qint64 bytesToWrite = static_cast<qint64>(frame.rgbaPixels.size());
        qint64 written = 0;
        while (written < bytesToWrite) {
            qint64 chunk = ffmpegProcess.write(pixelData + written, bytesToWrite - written);
            if (chunk < 0) break;
            written += chunk;
        }

        if (f % 5 == 0 || f == totalFrames - 1) {
            progress.setValue(f + 1);
            QCoreApplication::processEvents();
        }
    }

    ffmpegProcess.closeWriteChannel();
    ffmpegProcess.waitForFinished(60000);

    if (canceled) {
        QFile::remove(savePath);
        return;
    }

    if (ffmpegProcess.exitStatus() == QProcess::NormalExit && ffmpegProcess.exitCode() == 0) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Xuất video thành công");
        msgBox.setText(QString("Video đã được xuất thành công tới:\n%1").arg(savePath));
        QPushButton* openFolderBtn = msgBox.addButton("Mở thư mục", QMessageBox::ActionRole);
        QPushButton* closeBtn = msgBox.addButton("Đóng", QMessageBox::RejectRole);
        (void)closeBtn;
        msgBox.setDefaultButton(openFolderBtn);
        msgBox.exec();

        if (msgBox.clickedButton() == openFolderBtn) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(savePath).absolutePath()));
        }
    } else {
        QString errorLog = QString::fromUtf8(ffmpegProcess.readAllStandardError());
        QMessageBox::critical(this, "Lỗi xuất video",
            QString("Quá trình xuất video gặp lỗi:\n%1").arg(errorLog.right(300)));
    }
}

void MainWindow::onThemeToggleRequested() {
    auto currentMode = Theme::instance().mode();
    Theme::instance().setTheme(currentMode == ThemeMode::Dark ? ThemeMode::Light : ThemeMode::Dark);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Space) {
        engine_.togglePlay();
        previewPanel_->refresh();
        event->accept();
        return;
    }

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

    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        engine_.deleteSelectedClips();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_S && event->modifiers() == Qt::NoModifier) {
        engine_.splitAtPlayhead();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_D && (event->modifiers() & Qt::ControlModifier)) {
        engine_.duplicateSelectedClips();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_N && event->modifiers() == Qt::NoModifier) {
        engine_.toggleSnapping();
        timelinePanel_->refresh();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

} // namespace catchim::ui
#endif
