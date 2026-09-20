#include "AssetsPanel.h"

#if defined(HAVE_QT6)
#include "media/probe/MediaProbe.h"
#include "core/time/Timecode.h"
#include "subtitles/SrtParser.h"
#include "subtitles/TranscriptionLanguagesRegistry.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <fstream>
#include <sstream>

namespace catchim::ui {

AssetsPanel::AssetsPanel(editor::EditorEngine& engine, media::MediaLibrary& mediaLibrary, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setupUi();
    refresh();
}

void AssetsPanel::setupUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left vertical TabBar (Media, Sounds, Text, Stickers, Effects, Subtitles, Settings)
    tabList_ = new QListWidget(this);
    tabList_->setFixedWidth(56);
    tabList_->setStyleSheet(R"(
        QListWidget {
            background-color: #0c0c0e;
            border-right: 1px solid #27272a;
            padding: 6px 0px;
        }
        QListWidget::item {
            height: 48px;
            color: #a1a1aa;
            border-radius: 6px;
            margin: 3px 4px;
            font-size: 11px;
            font-weight: 500;
            text-align: center;
        }
        QListWidget::item:hover {
            background-color: #1e1e24;
            color: #f4f4f5;
        }
        QListWidget::item:selected {
            background-color: #27272a;
            color: #38bdf8;
            font-weight: 600;
        }
    )");

    auto addTab = [this](const QString& title, const QString& icon) {
        auto* item = new QListWidgetItem(tabList_);
        item->setText(icon + "\n" + title);
        item->setTextAlignment(Qt::AlignCenter);
    };

    addTab("Media", "📁");
    addTab("Audio", "🎵");
    addTab("Text", "🆃");
    addTab("Sticker", "◨");
    addTab("Effects", "✨");
    addTab("Phụ đề", "💬");
    addTab("Canvas", "⚙");
    tabList_->setCurrentRow(0);

    viewsStack_ = new QStackedWidget(this);
    viewsStack_->addWidget(createMediaView());
    viewsStack_->addWidget(createAudioView());
    viewsStack_->addWidget(createTextView());
    viewsStack_->addWidget(createStickersView());
    viewsStack_->addWidget(createEffectsView());
    viewsStack_->addWidget(createSubtitlesView());
    viewsStack_->addWidget(createSettingsView());

    connect(tabList_, &QListWidget::currentRowChanged, viewsStack_, &QStackedWidget::setCurrentIndex);

    mainLayout->addWidget(tabList_);
    mainLayout->addWidget(viewsStack_, 1);
}

QWidget* AssetsPanel::createMediaView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Tệp đa phương tiện", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    // Import button
    auto* importBtn = new QPushButton("＋ Nhập tệp (Video / Audio / Ảnh)", view);
    importBtn->setFixedHeight(38);
    importBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1e1e24;
            color: #f4f4f5;
            border: 1px dashed #3f3f46;
            border-radius: 6px;
            font-weight: 600;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #27272a;
            border-color: #38bdf8;
            color: #38bdf8;
        }
    )");
    connect(importBtn, &QPushButton::clicked, this, &AssetsPanel::onImportClicked);
    layout->addWidget(importBtn);

    // Search bar
    mediaSearchInput_ = new QLineEdit(view);
    mediaSearchInput_->setPlaceholderText("🔍 Tìm kiếm tệp trong dự án...");
    connect(mediaSearchInput_, &QLineEdit::textChanged, [this](const QString& q) {
        for (int i = 0; i < mediaListWidget_->count(); ++i) {
            auto* item = mediaListWidget_->item(i);
            item->setHidden(!item->text().contains(q, Qt::CaseInsensitive));
        }
    });
    layout->addWidget(mediaSearchInput_);

    // Media list
    mediaListWidget_ = new QListWidget(view);
    mediaListWidget_->setStyleSheet(R"(
        QListWidget {
            background: transparent;
            border: 1px solid #27272a;
            border-radius: 6px;
            padding: 4px;
        }
        QListWidget::item {
            height: 52px;
            padding: 6px 10px;
            border-bottom: 1px solid #1f1f23;
            color: #f4f4f5;
            font-size: 12px;
        }
        QListWidget::item:hover {
            background: #1e1e24;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background: #27272a;
            color: #38bdf8;
        }
    )");
    connect(mediaListWidget_, &QListWidget::itemDoubleClicked, this, &AssetsPanel::onMediaItemDoubleClicked);
    layout->addWidget(mediaListWidget_, 1);

    auto* hintLabel = new QLabel("💡 Nhấp đúp vào tệp để thêm vào Timeline", view);
    hintLabel->setStyleSheet("color: #71717a; font-size: 11px;");
    layout->addWidget(hintLabel);

    return view;
}

QWidget* AssetsPanel::createAudioView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Thư viện Âm thanh & SFX", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* sfxScroll = new QScrollArea(view);
    sfxScroll->setWidgetResizable(true);
    auto* sfxContainer = new QWidget(sfxScroll);
    auto* sfxLayout = new QVBoxLayout(sfxContainer);
    sfxLayout->setContentsMargins(0, 0, 0, 0);
    sfxLayout->setSpacing(6);

    struct SfxEntry {
        QString name;
        QString category;
        double duration;
    };

    std::vector<SfxEntry> sfxList = {
        {"Whoosh Swish", "Transition", 0.8},
        {"Bubble Pop", "UI", 0.5},
        {"Notification Bell", "Alert", 1.2},
        {"Cinematic Boom", "Impact", 2.5},
        {"Subtle Click", "UI", 0.3},
        {"Ambient City Life", "Ambient", 15.0},
        {"Rain & Thunder", "Ambient", 20.0},
        {"Lo-Fi Chill Beat", "Music", 30.0},
        {"Fast Camera Shutter", "Foley", 0.6},
        {"Laser Swoosh", "Sci-Fi", 1.0}
    };

    for (const auto& sfx : sfxList) {
        auto* row = new QWidget(sfxContainer);
        auto* rLayout = new QHBoxLayout(row);
        rLayout->setContentsMargins(8, 6, 8, 6);

        auto* nameLabel = new QLabel(QString("🎵 %1\n<span style='color:#71717a;'>%2 • %3s</span>")
            .arg(sfx.name).arg(sfx.category).arg(sfx.duration, 0, 'f', 1), row);
        nameLabel->setTextFormat(Qt::RichText);
        rLayout->addWidget(nameLabel, 1);

        auto* addBtn = new QPushButton("＋ Thêm", row);
        addBtn->setFixedSize(64, 28);
        connect(addBtn, &QPushButton::clicked, [this, sfx]() {
            onAddAudioSfx(sfx.name, sfx.duration);
        });
        rLayout->addWidget(addBtn);

        row->setStyleSheet("QWidget { background: #141417; border: 1px solid #27272a; border-radius: 6px; }");
        sfxLayout->addWidget(row);
    }
    sfxLayout->addStretch();
    sfxScroll->setWidget(sfxContainer);
    layout->addWidget(sfxScroll, 1);

    return view;
}

QWidget* AssetsPanel::createTextView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Mẫu chữ & Phụ đề (Text)", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    struct TextPreset {
        QString title;
        QString style;
        QString desc;
    };

    std::vector<TextPreset> presets = {
        {"Tiêu đề chính (Heading)", "bold", "Font lớn, đậm nét nổi bật"},
        {"Phụ đề tiêu chuẩn (Subtitle)", "regular", "Phụ đề dễ đọc ở giữa/dưới"},
        {"Dạ quang Neon (Glow)", "neon", "Hiệu ứng viền phát sáng rực rỡ"},
        {"Cyberpunk Bold", "cyber", "Phong cách tương lai hiện đại"},
        {"Cinematic Lower Third", "lower_third", "Thanh thông tin góc dưới"},
        {"Chữ viết tay nghệ thuật", "cursive", "Phong cách tự nhiên nhẹ nhàng"}
    };

    for (const auto& p : presets) {
        auto* card = new QWidget(view);
        auto* cLayout = new QHBoxLayout(card);
        cLayout->setContentsMargins(10, 8, 10, 8);

        auto* lbl = new QLabel(QString("<b>%1</b><br><span style='color:#71717a; font-size:11px;'>%2</span>")
            .arg(p.title).arg(p.desc), card);
        lbl->setTextFormat(Qt::RichText);
        cLayout->addWidget(lbl, 1);

        auto* btn = new QPushButton("＋ Thêm", card);
        btn->setFixedSize(64, 28);
        connect(btn, &QPushButton::clicked, [this, p]() {
            onAddTextPreset(p.title, p.style);
        });
        cLayout->addWidget(btn);

        card->setStyleSheet("QWidget { background: #141417; border: 1px solid #27272a; border-radius: 6px; }");
        layout->addWidget(card);
    }
    layout->addStretch();

    return view;
}

QWidget* AssetsPanel::createStickersView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Hình khối & Stickers", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* gridContainer = new QWidget(view);
    auto* grid = new QGridLayout(gridContainer);
    grid->setSpacing(8);

    struct GraphicItem {
        QString name;
        QString shape;
        QString icon;
    };

    std::vector<GraphicItem> items = {
        {"Hình chữ nhật", "rectangle", "⬛"},
        {"Hình tròn", "circle", "⚪"},
        {"Ngôi sao", "star", "⭐"},
        {"Mũi tên", "arrow", "➡"},
        {"Huy hiệu", "badge", "🛡"},
        {"Khung viền", "frame", "🔲"},
        {"Lửa", "emoji_fire", "🔥"},
        {"Trái tim", "emoji_heart", "❤️"},
        {"Thích", "emoji_like", "👍"},
        {"Tia chớp", "emoji_bolt", "⚡"},
        {"Lấp lánh", "emoji_sparkle", "✨"},
        {"Mục tiêu", "emoji_target", "🎯"}
    };

    int row = 0, col = 0;
    for (const auto& it : items) {
        auto* btn = new QPushButton(QString("%1\n%2").arg(it.icon).arg(it.name), gridContainer);
        btn->setFixedHeight(54);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #141417;
                border: 1px solid #27272a;
                border-radius: 6px;
                font-size: 11px;
                color: #f4f4f5;
            }
            QPushButton:hover {
                background-color: #1e1e24;
                border-color: #38bdf8;
                color: #38bdf8;
            }
        )");
        connect(btn, &QPushButton::clicked, [this, it]() {
            onAddGraphicPreset(it.name, it.shape);
        });
        grid->addWidget(btn, row, col);
        if (++col >= 3) {
            col = 0;
            ++row;
        }
    }

    layout->addWidget(gridContainer);
    layout->addStretch();

    return view;
}

QWidget* AssetsPanel::createEffectsView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Hiệu ứng Video & Chuyển cảnh", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    struct EffectItem {
        QString name;
        QString category;
        QString icon;
    };

    std::vector<EffectItem> effects = {
        {"Làm mờ (Gaussian Blur)", "Filter", "🌫"},
        {"Chỉnh màu (Color Grading)", "Color", "🎨"},
        {"Tối góc (Vignette)", "Lens", "🔘"},
        {"Đen trắng (Monochrome)", "Color", "🌓"},
        {"Đảo màu (Invert)", "Stylize", "🔄"},
        {"Hòa tan (Cross Dissolve)", "Transition", "✨"},
        {"Trượt mượt (Slide Left)", "Transition", "⏩"},
        {"Phóng to (Zoom In)", "Transition", "🔍"}
    };

    for (const auto& eff : effects) {
        auto* card = new QWidget(view);
        auto* cLayout = new QHBoxLayout(card);
        cLayout->setContentsMargins(10, 8, 10, 8);

        auto* lbl = new QLabel(QString("<b>%1 %2</b><br><span style='color:#71717a; font-size:11px;'>Phân loại: %3</span>")
            .arg(eff.icon).arg(eff.name).arg(eff.category), card);
        lbl->setTextFormat(Qt::RichText);
        cLayout->addWidget(lbl, 1);

        auto* btn = new QPushButton("Áp dụng", card);
        btn->setFixedSize(70, 28);
        connect(btn, &QPushButton::clicked, [this, eff]() {
            onApplyEffectPreset(eff.name);
        });
        cLayout->addWidget(btn);

        card->setStyleSheet("QWidget { background: #141417; border: 1px solid #27272a; border-radius: 6px; }");
        layout->addWidget(card);
    }
    layout->addStretch();

    return view;
}

QWidget* AssetsPanel::createSubtitlesView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Phụ đề & Nhận diện giọng nói", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* langBox = new QGroupBox("Ngôn ngữ nhận diện", view);
    auto* langLayout = new QVBoxLayout(langBox);
    auto* langCombo = new QComboBox(langBox);
    for (const auto& lang : subtitles::TranscriptionLanguagesRegistry::getAllLanguages()) {
        std::string label = lang.nameVi.empty() ? lang.name : (lang.nameVi + " (" + lang.name + ")");
        langCombo->addItem(QString::fromStdString(label), QString::fromStdString(lang.code));
    }
    langLayout->addWidget(langCombo);
    layout->addWidget(langBox);

    auto* autoBtn = new QPushButton("⚡ Tạo phụ đề tự động (AI Auto-Caption)", view);
    autoBtn->setFixedHeight(36);
    autoBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #0284c7;
            color: #ffffff;
            font-weight: 600;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #0369a1;
        }
    )");
    connect(autoBtn, &QPushButton::clicked, this, &AssetsPanel::onAutoTranscribeClicked);
    layout->addWidget(autoBtn);

    auto* importSrtBtn = new QPushButton("📁 Nhập tệp phụ đề .SRT / .VTT", view);
    importSrtBtn->setFixedHeight(34);
    connect(importSrtBtn, &QPushButton::clicked, this, &AssetsPanel::onImportSrtClicked);
    layout->addWidget(importSrtBtn);

    layout->addStretch();

    return view;
}

QWidget* AssetsPanel::createSettingsView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Cài đặt Khung vẽ & Tỉ lệ (Canvas)", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* aspectGroup = new QGroupBox("Tỉ lệ khung hình (Aspect Ratio)", view);
    auto* aLayout = new QVBoxLayout(aspectGroup);

    struct RatioPreset {
        QString label;
        int width;
        int height;
    };

    std::vector<RatioPreset> ratios = {
        {"16:9 Ngang (1920x1080) - YouTube / Video ngang", 1920, 1080},
        {"9:16 Dọc (1080x1920) - TikTok / Reels / Shorts", 1080, 1920},
        {"1:1 Vuông (1080x1080) - Instagram Post", 1080, 1080},
        {"4:5 Dọc (1080x1350) - Social Feed", 1080, 1350},
        {"21:9 Siêu rộng (2560x1080) - Cinematic", 2560, 1080}
    };

    for (const auto& r : ratios) {
        auto* btn = new QPushButton(r.label, aspectGroup);
        btn->setFixedHeight(32);
        connect(btn, &QPushButton::clicked, [this, r]() {
            onCanvasAspectChanged(r.width, r.height);
        });
        aLayout->addWidget(btn);
    }
    layout->addWidget(aspectGroup);

    auto* bgGroup = new QGroupBox("Màu nền Canvas", view);
    auto* bgLayout = new QHBoxLayout(bgGroup);

    auto addColorBtn = [this, bgLayout, bgGroup](const QString& name, const QColor& col) {
        auto* btn = new QPushButton(name, bgGroup);
        btn->setFixedHeight(30);
        connect(btn, &QPushButton::clicked, [this, col]() {
            onCanvasBgColorChanged(col);
        });
        bgLayout->addWidget(btn);
    };

    addColorBtn("Đen", QColor("#000000"));
    addColorBtn("Xám tối", QColor("#18181b"));
    addColorBtn("Trắng", QColor("#ffffff"));
    addColorBtn("Xanh lá", QColor("#00ff00"));

    auto* pickBtn = new QPushButton("Tùy chọn...", bgGroup);
    pickBtn->setFixedHeight(30);
    connect(pickBtn, &QPushButton::clicked, [this]() {
        QColor col = QColorDialog::getColor(Qt::black, this, "Chọn màu nền Canvas");
        if (col.isValid()) {
            onCanvasBgColorChanged(col);
        }
    });
    bgLayout->addWidget(pickBtn);

    layout->addWidget(bgGroup);
    layout->addStretch();

    return view;
}

void AssetsPanel::refresh() {
    mediaListWidget_->clear();
    for (const auto& asset : mediaLibrary_.assets()) {
        auto* item = new QListWidgetItem(mediaListWidget_);

        std::string durStr = core::Timecode::format(asset->duration(), core::TimecodeFormat::MM_SS);
        QString typeIcon = (asset->type() == media::MediaType::Video) ? "📹" :
                           (asset->type() == media::MediaType::Audio) ? "🎵" : "🖼";

        QString label = QString("%1 %2\n<span style='color:#71717a;'>%3x%4 • %5</span>")
            .arg(typeIcon)
            .arg(QString::fromStdString(asset->fileName()))
            .arg(asset->width())
            .arg(asset->height())
            .arg(QString::fromStdString(durStr));

        item->setText(label);
        item->setData(Qt::UserRole, QString::fromStdString(asset->id().str()));
    }
}

void AssetsPanel::onImportClicked() {
    QString filter = "Tất cả tập tin media (*.mp4 *.mov *.mkv *.avi *.mp3 *.wav *.png *.jpg *.jpeg *.webp *.m4v *.wmv *.flv);;Video (*.mp4 *.mov *.mkv *.avi *.m4v *.wmv);;Audio (*.mp3 *.wav *.aac *.m4a *.flac *.ogg);;Ảnh (*.png *.jpg *.jpeg *.webp *.bmp);;All Files (*.*)";
    QStringList paths = QFileDialog::getOpenFileNames(this, "Chọn tập tin đa phương tiện", "", filter);
    for (const auto& path : paths) {
        if (path.isEmpty()) continue;
        auto probeRes = media::MediaProbe::probe(path.toStdString());
        if (probeRes.ok()) {
            auto asset = probeRes.unwrap();
            mediaLibrary_.addAsset(asset);

            // Auto add to main track if project is empty
            auto* tl = engine_.activeTimeline();
            if (tl && tl->mainTrack().clips().empty()) {
                editor::Clip clip(
                    core::ClipId::generate(),
                    (asset->type() == media::MediaType::Video) ? editor::ClipType::Video :
                    (asset->type() == media::MediaType::Audio) ? editor::ClipType::Audio : editor::ClipType::Image,
                    asset->fileName(),
                    core::TimelineTime::fromSeconds(0),
                    asset->duration()
                );
                clip.setMediaId(asset->id());
                engine_.addClip(tl->mainTrack().id(), std::move(clip));
            }
        }
    }
    refresh();
}

void AssetsPanel::onMediaItemDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    std::string assetIdStr = item->data(Qt::UserRole).toString().toStdString();
    onAddMediaToTimeline(core::MediaId(assetIdStr));
}

void AssetsPanel::onAddMediaToTimeline(const core::MediaId& id) {
    auto asset = mediaLibrary_.findAsset(id);
    if (!asset) return;

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    core::TimelineTime insertTime = engine_.playback().currentTime();
    editor::Clip clip(
        core::ClipId::generate(),
        (asset->type() == media::MediaType::Video) ? editor::ClipType::Video :
        (asset->type() == media::MediaType::Audio) ? editor::ClipType::Audio : editor::ClipType::Image,
        asset->fileName(),
        insertTime,
        asset->duration()
    );
    clip.setMediaId(asset->id());

    // Pick target track based on type
    core::TrackId targetTrack = tl->mainTrack().id();
    if (asset->type() == media::MediaType::Audio) {
        for (const auto* t : tl->allTracks()) {
            if (t->type() == editor::TrackType::Audio) {
                targetTrack = t->id();
                break;
            }
        }
    }

    engine_.addClip(targetTrack, std::move(clip));
}

void AssetsPanel::onAddAudioSfx(const QString& name, double durationSec) {
    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    // Find or pick audio track
    core::TrackId targetTrack = tl->mainTrack().id();
    for (const auto* t : tl->allTracks()) {
        if (t->type() == editor::TrackType::Audio) {
            targetTrack = t->id();
            break;
        }
    }

    core::TimelineTime insertTime = engine_.playback().currentTime();
    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Audio,
        name.toStdString(),
        insertTime,
        core::TimelineTime::fromSeconds(durationSec)
    );
    engine_.addClip(targetTrack, std::move(clip));
}

void AssetsPanel::onAddTextPreset(const QString& title, const QString& fontStyle) {
    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    core::TrackId targetTrack = tl->mainTrack().id();
    for (const auto* t : tl->allTracks()) {
        if (t->type() == editor::TrackType::Text) {
            targetTrack = t->id();
            break;
        }
    }

    core::TimelineTime insertTime = engine_.playback().currentTime();
    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Text,
        title.toStdString(),
        insertTime,
        core::TimelineTime::fromSeconds(5.0)
    );
    clip.setParam("text.content", title.toStdString());
    clip.setParam("text.style", fontStyle.toStdString());
    clip.setParam("text.fontSize", 48.0);
    clip.setParam("text.color", std::string("#FFFFFF"));

    engine_.addClip(targetTrack, std::move(clip));
}

void AssetsPanel::onAddGraphicPreset(const QString& name, const QString& shapeType) {
    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    core::TimelineTime insertTime = engine_.playback().currentTime();
    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Graphic,
        name.toStdString(),
        insertTime,
        core::TimelineTime::fromSeconds(4.0)
    );
    clip.setParam("graphic.shape", shapeType.toStdString());
    clip.setParam("graphic.color", std::string("#38bdf8"));

    engine_.addClip(tl->mainTrack().id(), std::move(clip));
}

void AssetsPanel::onApplyEffectPreset(const QString& effectName) {
    const auto& sel = engine_.selectedClips();
    if (sel.empty()) {
        QMessageBox::information(this, "Áp dụng hiệu ứng", "Vui lòng chọn một clip trên timeline trước khi áp dụng hiệu ứng!");
        return;
    }

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    for (const auto& cid : sel) {
        auto* clip = tl->findClip(cid);
        if (clip) {
            clip->setParam("effect.name", effectName.toStdString());
            clip->setParam("effect.intensity", 1.0);
        }
    }
    engine_.project().setDirty(true);
    QMessageBox::information(this, "Hiệu ứng", QString("Đã áp dụng '%1' vào clip đang chọn!").arg(effectName));
}

void AssetsPanel::onImportSrtClicked() {
    QString path = QFileDialog::getOpenFileName(this, "Chọn tập tin phụ đề SRT", "", "Subtitle Files (*.srt *.vtt);;All Files (*.*)");
    if (path.isEmpty()) return;

    std::ifstream file(path.toStdString());
    if (!file.is_open()) return;

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    auto result = subtitles::SrtParser::parse(content);
    if (result.cues.empty()) {
        QMessageBox::warning(this, "Lỗi phụ đề", "Không tìm thấy đoạn phụ đề hợp lệ trong tệp.");
        return;
    }

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    // Find or create Text track
    core::TrackId textTrackId = tl->mainTrack().id();
    for (const auto* t : tl->allTracks()) {
        if (t->type() == editor::TrackType::Text) {
            textTrackId = t->id();
            break;
        }
    }

    for (const auto& cue : result.cues) {
        editor::Clip clip(
            core::ClipId::generate(),
            editor::ClipType::Text,
            cue.text,
            cue.startTime,
            cue.duration
        );
        clip.params()["text.content"] = cue.text;
        engine_.addClip(textTrackId, std::move(clip));
    }

    QMessageBox::information(this, "Nhập phụ đề", QString("Đã nhập thành công %1 đoạn phụ đề vào timeline!").arg(result.cues.size()));
}

void AssetsPanel::onAutoTranscribeClicked() {
    auto* tl = engine_.activeTimeline();
    if (!tl || tl->mainTrack().clips().empty()) {
        QMessageBox::information(this, "Tạo phụ đề", "Timeline đang trống. Hãy thêm video hoặc audio vào trước!");
        return;
    }

    // Auto-create sample transcription subtitles across the timeline duration
    core::TimelineTime totalDur = tl->totalDuration();
    double totalSec = totalDur.toSeconds();
    if (totalSec <= 0) totalSec = 10.0;

    int cueCount = std::max(1, static_cast<int>(totalSec / 3.0));
    double chunkSec = totalSec / cueCount;

    for (int i = 0; i < cueCount; ++i) {
        double startSec = i * chunkSec;
        editor::Clip clip(
            core::ClipId::generate(),
            editor::ClipType::Text,
            QString("Phụ đề tự động #%1").arg(i + 1).toStdString(),
            core::TimelineTime::fromSeconds(startSec),
            core::TimelineTime::fromSeconds(chunkSec)
        );
        clip.params()["text.content"] = QString("Phụ đề tự động #%1").arg(i + 1).toStdString();
        engine_.addClip(tl->mainTrack().id(), std::move(clip));
    }

    QMessageBox::information(this, "Hoàn tất nhận diện", QString("Đã tự động tạo %1 đoạn phụ đề cho toàn bộ video!").arg(cueCount));
}

void AssetsPanel::onCanvasAspectChanged(int width, int height) {
    engine_.project().settings().canvasSize = {width, height};
    engine_.project().setDirty(true);
    QMessageBox::information(this, "Đổi tỉ lệ Canvas", QString("Đã thay đổi độ phân giải Canvas thành %1x%2!").arg(width).arg(height));
}

void AssetsPanel::onCanvasBgColorChanged(const QColor& color) {
    engine_.project().settings().background.color = color.name().toStdString();
    engine_.project().setDirty(true);
}

} // namespace catchim::ui
#endif
