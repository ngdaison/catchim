#include "AssetsPanel.h"

#if defined(HAVE_QT6)
#include "media/probe/MediaProbe.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>

namespace catchim::ui {

AssetsPanel::AssetsPanel(editor::EditorEngine& engine, media::MediaLibrary& mediaLibrary, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
{
    setupUi();
    refresh();
}

void AssetsPanel::setupUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left vertical TabBar (Media, Sounds, Text, Stickers, Effects, Settings)
    tabList_ = new QListWidget(this);
    tabList_->setFixedWidth(44);
    tabList_->setStyleSheet(R"(
        QListWidget {
            background-color: #141414;
            border-right: 1px solid #292929;
            padding: 4px 0px;
        }
        QListWidget::item {
            height: 36px;
            color: #808080;
            border-radius: 4px;
            margin: 2px 4px;
        }
        QListWidget::item:hover {
            background-color: #242424;
            color: #DEDEDE;
        }
        QListWidget::item:selected {
            background-color: #00223D;
            color: #16A9F3;
        }
    )");

    new QListWidgetItem("📁", tabList_); // Media
    new QListWidgetItem("🎵", tabList_); // Sounds
    new QListWidgetItem("🆃", tabList_);  // Text
    new QListWidgetItem("✨", tabList_); // Effects
    new QListWidgetItem("⚙", tabList_);  // Settings
    tabList_->setCurrentRow(0);

    viewsStack_ = new QStackedWidget(this);

    // 1. Media View
    auto* mediaView = new QWidget(this);
    auto* mediaLayout = new QVBoxLayout(mediaView);
    mediaLayout->setContentsMargins(8, 8, 8, 8);
    mediaLayout->setSpacing(8);

    auto* importBtn = new QPushButton("+ Nhập tệp (Import)", mediaView);
    importBtn->setFixedHeight(32);
    importBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #242424;
            color: #DEDEDE;
            border: 1px dashed #404040;
            border-radius: 6px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: #2D2D2D;
            border-color: #16A9F3;
            color: #16A9F3;
        }
    )");
    connect(importBtn, &QPushButton::clicked, this, &AssetsPanel::onImportClicked);
    mediaLayout->addWidget(importBtn);

    mediaListWidget_ = new QListWidget(mediaView);
    mediaListWidget_->setStyleSheet(R"(
        QListWidget {
            background: transparent;
            border: none;
        }
        QListWidget::item {
            height: 48px;
            padding: 4px 8px;
            border-bottom: 1px solid #222222;
            color: #DEDEDE;
        }
        QListWidget::item:hover {
            background: #242424;
            border-radius: 4px;
        }
    )");
    connect(mediaListWidget_, &QListWidget::itemDoubleClicked, this, &AssetsPanel::onMediaItemDoubleClicked);
    mediaLayout->addWidget(mediaListWidget_);

    viewsStack_->addWidget(mediaView);

    // 2. Sounds View
    auto* soundsView = new QWidget(this);
    auto* sLayout = new QVBoxLayout(soundsView);
    sLayout->addWidget(new QLabel("Thư viện hiệu ứng âm thanh (Sounds)", soundsView));
    viewsStack_->addWidget(soundsView);

    // 3. Text View
    auto* textView = new QWidget(this);
    auto* tLayout = new QVBoxLayout(textView);
    auto* addTextBtn = new QPushButton("+ Thêm phụ đề / Chữ", textView);
    addTextBtn->setFixedHeight(32);
    addTextBtn->setStyleSheet(importBtn->styleSheet());
    connect(addTextBtn, &QPushButton::clicked, [this]() {
        auto* tl = engine_.activeTimeline();
        if (!tl) return;
        auto* track = tl->allTracks()[0];
        if (track) {
            core::ClipId cid = core::ClipId::generate();
            editor::Clip textClip(cid, editor::ClipType::Text, "Tiêu đề mẫu", core::TimelineTime::fromSeconds(0), core::TimelineTime::fromSeconds(5));
            engine_.addClip(track->id(), std::move(textClip));
        }
    });
    tLayout->addWidget(addTextBtn);
    tLayout->addStretch();
    viewsStack_->addWidget(textView);

    // 4. Effects View
    auto* effectsView = new QWidget(this);
    auto* eLayout = new QVBoxLayout(effectsView);
    eLayout->addWidget(new QLabel("Hiệu ứng Video (Blur, Color Grading, Vignette)", effectsView));
    viewsStack_->addWidget(effectsView);

    // 5. Settings View
    auto* settingsView = new QWidget(this);
    auto* settsLayout = new QVBoxLayout(settingsView);
    settsLayout->addWidget(new QLabel("Cài đặt Canvas và Background", settingsView));
    viewsStack_->addWidget(settingsView);

    connect(tabList_, &QListWidget::currentRowChanged, viewsStack_, &QStackedWidget::setCurrentIndex);

    mainLayout->addWidget(tabList_);
    mainLayout->addWidget(viewsStack_);
}

void AssetsPanel::refresh() {
    mediaListWidget_->clear();
    for (const auto& asset : mediaLibrary_.assets()) {
        auto* item = new QListWidgetItem(mediaListWidget_);
        item->setText(QString::fromStdString(asset->fileName()));
        item->setData(Qt::UserRole, QString::fromStdString(asset->id().str()));
    }
}

void AssetsPanel::onImportClicked() {
    QString filter = "Media Files (*.mp4 *.mov *.mkv *.avi *.mp3 *.wav *.png *.jpg *.jpeg);;All Files (*.*)";
    QString path = QFileDialog::getOpenFileName(this, "Chọn tập tin media", "", filter);
    if (!path.isEmpty()) {
        auto probeRes = media::MediaProbe::probe(path.toStdString());
        if (probeRes.ok()) {
            auto asset = probeRes.unwrap();
            mediaLibrary_.addAsset(asset);
            refresh();

            // Auto-add to main track if empty
            auto* tl = engine_.activeTimeline();
            if (tl && tl->mainTrack().clips().empty()) {
                editor::Clip clip(
                    core::ClipId::generate(),
                    (asset->type() == media::MediaType::Video) ? editor::ClipType::Video : editor::ClipType::Audio,
                    asset->fileName(),
                    core::TimelineTime::fromSeconds(0),
                    asset->duration()
                );
                clip.setMediaId(asset->id());
                engine_.addClip(tl->mainTrack().id(), std::move(clip));
            }
        }
    }
}

void AssetsPanel::onMediaItemDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    std::string assetIdStr = item->data(Qt::UserRole).toString().toStdString();
    auto asset = mediaLibrary_.findAsset(core::MediaId(assetIdStr));
    if (!asset) return;

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    core::TimelineTime insertTime = tl->totalDuration();
    editor::Clip clip(
        core::ClipId::generate(),
        (asset->type() == media::MediaType::Video) ? editor::ClipType::Video : editor::ClipType::Audio,
        asset->fileName(),
        insertTime,
        asset->duration()
    );
    clip.setMediaId(asset->id());
    engine_.addClip(tl->mainTrack().id(), std::move(clip));
}

} // namespace catchim::ui
#endif
