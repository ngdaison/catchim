#include "ExportDialog.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QApplication>
#include <cmath>

namespace catchim::ui {

ExportDialog::ExportDialog(
    editor::EditorEngine& engine,
    media::MediaLibrary& mediaLibrary,
    render::RenderEngine& renderEngine,
    QWidget* parent
)
    : QDialog(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
    , renderEngine_(renderEngine)
{
    setWindowTitle("Xuất dự án (Export)");
    setFixedSize(440, 360);
    setStyleSheet(R"(
        QDialog {
            background-color: #0c0c0e;
            color: #f4f4f5;
        }
        QLabel {
            color: #f4f4f5;
            font-size: 13px;
        }
        QComboBox {
            background-color: #18181b;
            color: #f4f4f5;
            border: 1px solid #27272a;
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 13px;
        }
        QComboBox:hover {
            border-color: #38bdf8;
        }
        QComboBox::drop-down {
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #18181b;
            color: #f4f4f5;
            border: 1px solid #27272a;
            selection-background-color: #27272a;
            selection-color: #38bdf8;
        }
    )");

    setupUi();
    updateInfoLabel();
}

ExportDialog::~ExportDialog() {
    if (ffmpegProcess_ && ffmpegProcess_->state() == QProcess::Running) {
        ffmpegProcess_->kill();
        ffmpegProcess_->waitForFinished(1000);
    }
}

void ExportDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(16);

    // Title & Icon
    auto* headerLayout = new QHBoxLayout();
    auto* titleIcon = new QLabel(this);
    titleIcon->setPixmap(UiIcons::getPixmap(UiIcon::Export, QColor("#38bdf8"), 24));
    auto* titleLabel = new QLabel("Cấu hình kết xuất video", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: 700; color: #f4f4f5;");
    headerLayout->addWidget(titleIcon);
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // Settings Area
    settingsWidget_ = new QWidget(this);
    auto* formLayout = new QFormLayout(settingsWidget_);
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    resolutionCombo_ = new QComboBox(settingsWidget_);
    resolutionCombo_->addItem("1080p FHD (1920x1080)", "1080p");
    resolutionCombo_->addItem("720p HD (1280x720)", "720p");
    resolutionCombo_->addItem("480p SD (854x480)", "480p");
    resolutionCombo_->addItem("1440p 2K (2560x1440)", "1440p");
    resolutionCombo_->addItem("2160p 4K (3840x2160)", "2160p");
    resolutionCombo_->addItem("Gốc dự án (Source Canvas)", "source");
    connect(resolutionCombo_, &QComboBox::currentIndexChanged, this, &ExportDialog::onResolutionChanged);
    formLayout->addRow("Độ phân giải:", resolutionCombo_);

    formatCombo_ = new QComboBox(settingsWidget_);
    formatCombo_->addItem("MP4 (H.264 / AAC)", "mp4");
    formatCombo_->addItem("WebM (VP9 / Vorbis)", "webm");
    connect(formatCombo_, &QComboBox::currentIndexChanged, this, &ExportDialog::onFormatChanged);
    formLayout->addRow("Định dạng:", formatCombo_);

    qualityCombo_ = new QComboBox(settingsWidget_);
    qualityCombo_->addItem("Cao (High)", "high");
    qualityCombo_->addItem("Trung bình (Medium)", "medium");
    qualityCombo_->addItem("Rất cao (Very High)", "very_high");
    qualityCombo_->addItem("Thấp (Low)", "low");
    formLayout->addRow("Chất lượng:", qualityCombo_);

    audioCombo_ = new QComboBox(settingsWidget_);
    audioCombo_->addItem("Bao gồm âm thanh", "include");
    audioCombo_->addItem("Tắt tiếng (Mute)", "mute");
    formLayout->addRow("Âm thanh:", audioCombo_);

    mainLayout->addWidget(settingsWidget_);

    // Info Label
    infoLabel_ = new QLabel(this);
    infoLabel_->setStyleSheet("color: #71717a; font-size: 12px;");
    mainLayout->addWidget(infoLabel_);

    // Progress Area (hidden by default)
    progressWidget_ = new QWidget(this);
    auto* progLayout = new QVBoxLayout(progressWidget_);
    progLayout->setContentsMargins(0, 0, 0, 0);
    progLayout->setSpacing(8);

    progressStatusLabel_ = new QLabel("Đang chuẩn bị kết xuất...", progressWidget_);
    progressStatusLabel_->setStyleSheet("font-size: 12px; color: #a1a1aa;");
    progLayout->addWidget(progressStatusLabel_);

    progressBar_ = new QProgressBar(progressWidget_);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setFixedHeight(18);
    progressBar_->setStyleSheet(R"(
        QProgressBar {
            background-color: #18181b;
            border: 1px solid #27272a;
            border-radius: 4px;
            text-align: center;
            color: #f4f4f5;
            font-size: 11px;
            font-weight: 600;
        }
        QProgressBar::chunk {
            background-color: #0284c7;
            border-radius: 3px;
        }
    )");
    progLayout->addWidget(progressBar_);
    progressWidget_->hide();
    mainLayout->addWidget(progressWidget_);

    // Inline Status Banner
    statusBanner_ = new QLabel(this);
    statusBanner_->setVisible(false);
    statusBanner_->setWordWrap(true);
    mainLayout->addWidget(statusBanner_);

    mainLayout->addStretch();

    // Buttons
    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    cancelBtn_ = new QPushButton("Đóng", this);
    cancelBtn_->setFixedHeight(34);
    cancelBtn_->setStyleSheet(R"(
        QPushButton {
            background: #18181b;
            color: #f4f4f5;
            border: 1px solid #27272a;
            border-radius: 6px;
            padding: 0 16px;
            font-weight: 600;
        }
        QPushButton:hover {
            background: #27272a;
        }
    )");
    connect(cancelBtn_, &QPushButton::clicked, this, &ExportDialog::onCancelExport);
    btnLayout->addWidget(cancelBtn_);

    exportBtn_ = new QPushButton("Xuất video", this);
    exportBtn_->setFixedHeight(34);
    exportBtn_->setIcon(UiIcons::get(UiIcon::Export, QColor("#ffffff"), 16));
    exportBtn_->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #38bdf8);
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 0 20px;
            font-weight: 700;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0369a1, stop:1 #0284c7);
        }
    )");
    connect(exportBtn_, &QPushButton::clicked, this, &ExportDialog::onStartExport);
    btnLayout->addWidget(exportBtn_);

    mainLayout->addLayout(btnLayout);
}

QSize ExportDialog::getTargetResolution() const {
    QString resCode = resolutionCombo_->currentData().toString();
    int srcW = engine_.project().settings().canvasSize.width;
    int srcH = engine_.project().settings().canvasSize.height;
    if (srcW <= 0 || srcH <= 0) {
        srcW = 1920;
        srcH = 1080;
    }

    if (resCode == "480p") return QSize(854, 480);
    if (resCode == "720p") return QSize(1280, 720);
    if (resCode == "1080p") return QSize(1920, 1080);
    if (resCode == "1440p") return QSize(2560, 1440);
    if (resCode == "2160p") return QSize(3840, 2160);
    return QSize(srcW, srcH);
}

void ExportDialog::updateInfoLabel() {
    QSize sz = getTargetResolution();
    const auto& fpsSetting = engine_.project().settings().fps;
    double fps = (fpsSetting.denominator > 0) ?
        static_cast<double>(fpsSetting.numerator) / fpsSetting.denominator : 30.0;
    if (fps <= 0.0) fps = 30.0;

    infoLabel_->setText(QString("Kích thước xuất: %1 x %2 • Tốc độ khung hình: %3 FPS")
        .arg(sz.width()).arg(sz.height()).arg(fps, 0, 'f', 0));
}

void ExportDialog::onResolutionChanged() {
    updateInfoLabel();
}

void ExportDialog::onFormatChanged() {
    updateInfoLabel();
}

void ExportDialog::onCancelExport() {
    if (isExporting_) {
        cancelRequested_ = true;
        if (ffmpegProcess_ && ffmpegProcess_->state() == QProcess::Running) {
            ffmpegProcess_->kill();
        }
        isExporting_ = false;
        progressStatusLabel_->setText("Đã hủy kết xuất.");
        exportBtn_->setEnabled(true);
        cancelBtn_->setText("Đóng");
    } else {
        reject();
    }
}

void ExportDialog::onStartExport() {
    auto* tl = engine_.activeTimeline();
    if (!tl || tl->allTracks().empty()) {
        showStatusMessage("Dự án hiện chưa có phần tử nào trên Timeline.", "warning");
        return;
    }

    double totalSec = tl->totalDuration().toSeconds();
    if (totalSec <= 0.01) {
        showStatusMessage("Thời lượng Timeline quá ngắn để xuất.", "warning");
        return;
    }

    QString fmt = formatCombo_->currentData().toString();
    QString ext = (fmt == "webm") ? "webm" : "mp4";
    QString defaultName = QString::fromStdString(engine_.project().name()) + "." + ext;

    QString savePath = QFileDialog::getSaveFileName(
        this,
        "Chọn nơi lưu video",
        defaultName,
        QString("Video (*.%1);;All Files (*.*)").arg(ext)
    );
    if (savePath.isEmpty()) return;

    QSize targetSize = getTargetResolution();
    int width = targetSize.width();
    int height = targetSize.height();

    const auto& fpsSetting = engine_.project().settings().fps;
    double fps = (fpsSetting.denominator > 0) ?
        static_cast<double>(fpsSetting.numerator) / fpsSetting.denominator : 30.0;
    if (fps <= 0.0) fps = 30.0;

    int totalFrames = static_cast<int>(std::ceil(totalSec * fps));
    if (totalFrames <= 0) totalFrames = 1;

    QString quality = qualityCombo_->currentData().toString();
    QString crf = "23";
    if (quality == "low") crf = "28";
    else if (quality == "medium") crf = "23";
    else if (quality == "high") crf = "18";
    else if (quality == "very_high") crf = "14";

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
         << "-i" << "-";

    if (fmt == "webm") {
        args << "-c:v" << "libvpx-vp9"
             << "-pix_fmt" << "yuva420p"
             << "-crf" << crf
             << "-b:v" << "0";
    } else {
        args << "-c:v" << "libx264"
             << "-pix_fmt" << "yuv420p"
             << "-preset" << "fast"
             << "-crf" << crf;
    }
    args << savePath;

    ffmpegProcess_ = std::make_unique<QProcess>();
    ffmpegProcess_->start(ffmpegPath, args);
    if (!ffmpegProcess_->waitForStarted(5000)) {
        showStatusMessage("Không thể khởi chạy tiến trình FFmpeg.\nVui lòng kiểm tra FFmpeg đã được cài đặt trong hệ thống.", "error");
        return;
    }

    // Switch UI to exporting state
    isExporting_ = true;
    cancelRequested_ = false;
    settingsWidget_->setEnabled(false);
    progressWidget_->show();
    exportBtn_->setEnabled(false);
    cancelBtn_->setText("Hủy kết xuất");

    renderEngine_.setCanvasSize(width, height);

    for (int f = 0; f < totalFrames; ++f) {
        if (cancelRequested_) {
            ffmpegProcess_->kill();
            break;
        }

        double timeSec = static_cast<double>(f) / fps;
        const auto& output = renderEngine_.renderFrame(
            engine_.project(),
            mediaLibrary_,
            core::TimelineTime::fromSeconds(timeSec)
        );

        if (output.rgbaPixels.size() == static_cast<size_t>(width * height * 4)) {
            const char* dataPtr = reinterpret_cast<const char*>(output.rgbaPixels.data());
            qint64 toWrite = static_cast<qint64>(output.rgbaPixels.size());
            qint64 written = 0;
            while (written < toWrite && ffmpegProcess_->state() == QProcess::Running) {
                qint64 w = ffmpegProcess_->write(dataPtr + written, toWrite - written);
                if (w <= 0) {
                    if (!ffmpegProcess_->waitForBytesWritten(100)) break;
                } else {
                    written += w;
                }
            }
        }

        int pct = static_cast<int>(std::round((static_cast<double>(f + 1) / totalFrames) * 100.0));
        progressBar_->setValue(pct);
        progressStatusLabel_->setText(QString("Đang kết xuất khung hình %1 / %2 (%3%)...")
            .arg(f + 1).arg(totalFrames).arg(pct));
        QApplication::processEvents();
    }

    if (!cancelRequested_) {
        ffmpegProcess_->closeWriteChannel();
        ffmpegProcess_->waitForFinished(15000);
        isExporting_ = false;
        progressBar_->setValue(100);
        progressStatusLabel_->setText("Xuất video thành công!");
        showStatusMessage(QString("Đã xuất video thành công vào:\n%1").arg(savePath), "success");
        cancelBtn_->setText("Hoàn tất");
        cancelBtn_->setEnabled(true);
    }
}

void ExportDialog::showStatusMessage(const QString& msg, const QString& type) {
    if (!statusBanner_) return;
    statusBanner_->setText(msg);
    if (type == "error") {
        statusBanner_->setStyleSheet("background: #450a0a; border: 1px solid #b91c1c; color: #fca5a5; padding: 8px 12px; border-radius: 6px; font-size: 12px; font-weight: 500;");
    } else if (type == "warning") {
        statusBanner_->setStyleSheet("background: #422006; border: 1px solid #d97706; color: #fde68a; padding: 8px 12px; border-radius: 6px; font-size: 12px; font-weight: 500;");
    } else {
        statusBanner_->setStyleSheet("background: #052e16; border: 1px solid #16a34a; color: #86efac; padding: 8px 12px; border-radius: 6px; font-size: 12px; font-weight: 600;");
    }
    statusBanner_->setVisible(true);
}

} // namespace catchim::ui
#endif
