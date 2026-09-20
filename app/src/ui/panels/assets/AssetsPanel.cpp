#include "AssetsPanel.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include "media/probe/MediaProbe.h"
#include "core/time/Timecode.h"
#include "subtitles/SrtParser.h"
#include "subtitles/TranscriptionLanguagesRegistry.h"
#include "audio/TtsEngine.h"
#include "audio/TtsServiceEngine.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QColorDialog>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QTextEdit>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QDateTime>
#include <fstream>
#include <sstream>
#include <cmath>

namespace catchim::ui {

AssetsPanel::AssetsPanel(editor::EditorEngine& engine, media::MediaLibrary& mediaLibrary, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
    , mediaLibrary_(mediaLibrary)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAcceptDrops(true);
    setupUi();
    refresh();
}

void AssetsPanel::setupUi() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Left vertical TabBar (Media, Audio, Text, Stickers, Effects, Subtitles, Settings)
    tabList_ = new QListWidget(this);
    tabList_->setFixedWidth(44);
    tabList_->setIconSize(QSize(20, 20));
    tabList_->setFocusPolicy(Qt::NoFocus);
    tabList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabList_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tabList_->setStyleSheet(R"(
        QListWidget {
            background-color: #0c0c0e;
            border: none;
            border-right: 1px solid #27272a;
            padding: 6px 0px;
        }
        QListWidget::item {
            height: 36px;
            width: 36px;
            border-radius: 6px;
            margin: 2px 4px;
        }
        QListWidget::item:hover {
            background-color: #1e1e24;
        }
        QListWidget::item:selected {
            background-color: #27272a;
        }
    )");

    static const std::vector<std::pair<QString, UiIcon>> kTabDefs = {
        {"Tệp đa phương tiện (Media)", UiIcon::Media},
        {"Âm thanh & SFX (Audio)", UiIcon::Audio},
        {"Văn bản & Tiêu đề (Text)", UiIcon::Text},
        {"Nhãn dán (Stickers)", UiIcon::Stickers},
        {"Hiệu ứng video (Effects)", UiIcon::Effects},
        {"Chuyển tiếp (Transitions)", UiIcon::Transitions},
        {"Phụ đề & Tự động nhận diện (Captions)", UiIcon::Captions},
        {"Lớp điều chỉnh màu (Adjustment)", UiIcon::Adjustment},
        {"Cài đặt khung hình (Canvas Settings)", UiIcon::Settings}
    };

    for (size_t i = 0; i < kTabDefs.size(); ++i) {
        auto* item = new QListWidgetItem(tabList_);
        item->setIcon(UiIcons::get(kTabDefs[i].second, (i == 0) ? QColor("#38bdf8") : QColor("#a1a1aa"), 20));
        item->setToolTip(kTabDefs[i].first);
        item->setSizeHint(QSize(36, 36));
        item->setTextAlignment(Qt::AlignCenter);
    }
    tabList_->setCurrentRow(0);

    viewsStack_ = new QStackedWidget(this);
    viewsStack_->addWidget(createMediaView());
    viewsStack_->addWidget(createAudioView());
    viewsStack_->addWidget(createTextView());
    viewsStack_->addWidget(createStickersView());
    viewsStack_->addWidget(createEffectsView());
    viewsStack_->addWidget(createTransitionsView());
    viewsStack_->addWidget(createSubtitlesView());
    viewsStack_->addWidget(createAdjustmentView());
    viewsStack_->addWidget(createSettingsView());

    connect(tabList_, &QListWidget::currentRowChanged, [this](int row) {
        viewsStack_->setCurrentIndex(row);
        for (int i = 0; i < tabList_->count(); ++i) {
            auto* it = tabList_->item(i);
            if (it && i < static_cast<int>(kTabDefs.size())) {
                QColor col = (i == row) ? QColor("#38bdf8") : QColor("#a1a1aa");
                it->setIcon(UiIcons::get(kTabDefs[i].second, col, 20));
            }
        }
    });

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
    auto* importBtn = new QPushButton("Nhập tệp (Video / Audio / Ảnh)", view);
    importBtn->setIcon(UiIcons::get(UiIcon::Plus, QColor("#38bdf8"), 16));
    importBtn->setIconSize(QSize(16, 16));
    importBtn->setFixedHeight(38);
    importBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #1e1e24;
            color: #f4f4f5;
            border: 1px dashed #3f3f46;
            border-radius: 6px;
            font-weight: 600;
            font-size: 12px;
            padding: 0px 12px;
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
    mediaSearchInput_->setPlaceholderText("Tìm kiếm tệp trong dự án...");
    mediaSearchInput_->setFixedHeight(34);
    mediaSearchInput_->setStyleSheet(R"(
        QLineEdit {
            background-color: #141417;
            color: #f4f4f5;
            border: 1px solid #27272a;
            border-radius: 6px;
            padding: 4px 10px;
            font-size: 12px;
        }
        QLineEdit:focus {
            border-color: #38bdf8;
        }
    )");
    connect(mediaSearchInput_, &QLineEdit::textChanged, [this](const QString& q) {
        for (int i = 0; i < mediaListWidget_->count(); ++i) {
            auto* item = mediaListWidget_->item(i);
            item->setHidden(!item->text().contains(q, Qt::CaseInsensitive));
        }
    });
    layout->addWidget(mediaSearchInput_);

    // Media list
    mediaListWidget_ = new QListWidget(view);
    mediaListWidget_->setAcceptDrops(false);
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

    auto* hintLabel = new QLabel("Nhấp đúp vào tệp để thêm vào Timeline", view);
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

        auto* ic = new QLabel(row);
        ic->setPixmap(UiIcons::getPixmap(UiIcon::Audio, QColor("#38bdf8"), 16));
        ic->setFixedSize(20, 20);
        rLayout->addWidget(ic);

        auto* nameLabel = new QLabel(QString("<b>%1</b><br><span style='color:#71717a;'>%2 • %3s</span>")
            .arg(sfx.name).arg(sfx.category).arg(sfx.duration, 0, 'f', 1), row);
        nameLabel->setTextFormat(Qt::RichText);
        rLayout->addWidget(nameLabel, 1);

        auto* addBtn = new QPushButton("Thêm", row);
        addBtn->setIcon(UiIcons::get(UiIcon::Plus, QColor("#f4f4f5"), 12));
        addBtn->setIconSize(QSize(12, 12));
        addBtn->setFixedSize(68, 28);
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
    layout->setSpacing(8);

    auto* titleLabel = new QLabel("Mẫu chữ & Giọng đọc AI (Text / TTS)", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* scroll = new QScrollArea(view);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto* container = new QWidget(scroll);
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(0, 0, 8, 0);
    cLayout->setSpacing(10);

    // Section 1: Text Presets
    auto* presetsHeader = new QLabel("MẪU VĂN BẢN (TEXT PRESETS)", container);
    presetsHeader->setStyleSheet("font-size: 11px; font-weight: 700; color: #38bdf8; text-transform: uppercase; letter-spacing: 0.5px;");
    cLayout->addWidget(presetsHeader);

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
        auto* card = new QWidget(container);
        auto* cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(10, 8, 10, 8);

        auto* lbl = new QLabel(QString("<b>%1</b><br><span style='color:#71717a; font-size:11px;'>%2</span>")
            .arg(p.title).arg(p.desc), card);
        lbl->setTextFormat(Qt::RichText);
        cardLayout->addWidget(lbl, 1);

        auto* btn = new QPushButton("Thêm", card);
        btn->setIcon(UiIcons::get(UiIcon::Plus, QColor("#f4f4f5"), 12));
        btn->setIconSize(QSize(12, 12));
        btn->setFixedSize(68, 28);
        connect(btn, &QPushButton::clicked, [this, p]() {
            onAddTextPreset(p.title, p.style);
        });
        cardLayout->addWidget(btn);

        card->setStyleSheet("QWidget { background: #141417; border: 1px solid #27272a; border-radius: 6px; }");
        cLayout->addWidget(card);
    }

    // Section 2: AI Voiceover (Text-to-Speech)
    auto* ttsHeader = new QLabel("GIỌNG ĐỌC AI (TEXT TO SPEECH)", container);
    ttsHeader->setStyleSheet("font-size: 11px; font-weight: 700; color: #38bdf8; text-transform: uppercase; letter-spacing: 0.5px; padding-top: 8px;");
    cLayout->addWidget(ttsHeader);

    auto* ttsCard = new QWidget(container);
    auto* ttsLayout = new QVBoxLayout(ttsCard);
    ttsLayout->setContentsMargins(10, 10, 10, 10);
    ttsLayout->setSpacing(8);
    ttsCard->setStyleSheet("background: #141417; border: 1px solid #27272a; border-radius: 6px;");

    auto* ttsInput = new QTextEdit(ttsCard);
    ttsInput->setPlaceholderText("Nhập văn bản cần đọc thành tiếng...");
    ttsInput->setPlainText("Chào mừng bạn đến với trình chỉnh sửa video Catchim.");
    ttsInput->setFixedHeight(65);
    ttsInput->setStyleSheet("background: #1e1e24; color: #f4f4f5; border: 1px solid #27272a; border-radius: 4px; font-size: 12px; padding: 4px;");
    ttsLayout->addWidget(ttsInput);

    auto* voiceCombo = new QComboBox(ttsCard);
    voiceCombo->setStyleSheet("background: #1e1e24; color: #f4f4f5; border: 1px solid #27272a; border-radius: 4px; padding: 4px 8px; font-size: 11px;");
    for (const auto& v : audio::TtsEngine::instance().voices()) {
        QString label = QString::fromStdString(v.name + " (" + v.gender + " - " + v.language + ")");
        voiceCombo->addItem(label, QString::fromStdString(v.id));
    }
    ttsLayout->addWidget(voiceCombo);

    auto* rateLayout = new QHBoxLayout();
    auto* rateTitle = new QLabel("Tốc độ đọc:", ttsCard);
    rateTitle->setStyleSheet("color: #a1a1aa; font-size: 11px;");
    rateLayout->addWidget(rateTitle);

    auto* rateSlider = new QSlider(Qt::Horizontal, ttsCard);
    rateSlider->setRange(50, 200);
    rateSlider->setValue(100);
    rateLayout->addWidget(rateSlider, 1);

    auto* rateLabel = new QLabel("1.0x", ttsCard);
    rateLabel->setFixedWidth(30);
    rateLabel->setStyleSheet("color: #f4f4f5; font-size: 11px; font-weight: 600;");
    rateLayout->addWidget(rateLabel);
    connect(rateSlider, &QSlider::valueChanged, [rateLabel](int val) {
        rateLabel->setText(QString("%1x").arg(val / 100.0, 0, 'f', 1));
    });
    ttsLayout->addLayout(rateLayout);

    auto* genTtsBtn = new QPushButton("Tạo giọng nói vào Timeline", ttsCard);
    genTtsBtn->setIcon(UiIcons::get(UiIcon::Audio, QColor("#ffffff"), 16));
    genTtsBtn->setIconSize(QSize(16, 16));
    genTtsBtn->setFixedHeight(32);
    genTtsBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #0284c7;
            color: #ffffff;
            font-weight: 600;
            border-radius: 6px;
            font-size: 12px;
        }
        QPushButton:hover {
            background-color: #0369a1;
        }
    )");

    connect(genTtsBtn, &QPushButton::clicked, [this, ttsInput, voiceCombo, rateSlider]() {
        QString text = ttsInput->toPlainText().trimmed();
        if (text.isEmpty()) return;

        double rateMultiplier = rateSlider->value() / 100.0;
        double durationSec = audio::TtsEngine::estimateSpeechDuration(text.toStdString(), rateMultiplier, "vi");
        if (durationSec < 0.5) durationSec = 0.5;

        auto* tl = engine_.activeTimeline();
        if (!tl) return;

        core::TrackId audioTrackId = tl->mainTrack().id();
        for (const auto* t : tl->allTracks()) {
            if (t->type() == editor::TrackType::Audio) {
                audioTrackId = t->id();
                break;
            }
        }

        // 1. Synthesize voiced speech audio samples
        constexpr uint32_t sampleRate = 44100;
        size_t numSamples = static_cast<size_t>(durationSec * sampleRate);
        std::vector<float> monoSamples(numSamples, 0.0f);

        std::string vId = voiceCombo->currentData().toString().toStdString();
        bool isFemale = (vId.find("female") != std::string::npos);
        double f0 = isFemale ? 220.0 : 130.0;
        double f1 = isFemale ? 600.0 : 500.0;
        double f2 = isFemale ? 1800.0 : 1500.0;
        constexpr double kPi = 3.14159265358979323846;

        for (size_t i = 0; i < numSamples; ++i) {
            double t = static_cast<double>(i) / sampleRate;
            double env = 1.0;
            if (t < 0.05) {
                env = t / 0.05;
            } else if (t > durationSec - 0.05) {
                env = std::max(0.0, (durationSec - t) / 0.05);
            }

            double cadence = 0.6 + 0.4 * std::sin(2.0 * kPi * 4.0 * t);
            double s0 = std::sin(2.0 * kPi * f0 * t);
            double s1 = 0.5 * std::sin(2.0 * kPi * f1 * t);
            double s2 = 0.25 * std::sin(2.0 * kPi * f2 * t);
            monoSamples[i] = static_cast<float>((s0 * 0.5 + s1 * 0.3 + s2 * 0.2) * env * cadence * 0.7);
        }

        // 2. Encode to physical WAV bytes
        auto wavBytes = audio::TtsServiceEngine::encodePcmToWav(monoSamples, sampleRate);

        // 3. Save to disk cache
        QString cacheDir = QDir::tempPath() + "/catchim_audio_cache";
        QDir().mkpath(cacheDir);
        QString wavFilePath = QString("%1/tts_%2.wav").arg(cacheDir).arg(QDateTime::currentMSecsSinceEpoch());

        std::ofstream wavOut(wavFilePath.toStdString(), std::ios::binary);
        if (wavOut.is_open()) {
            wavOut.write(reinterpret_cast<const char*>(wavBytes.data()), static_cast<std::streamsize>(wavBytes.size()));
            wavOut.close();
        }

        // 4. Probe media file and register in media library
        core::MediaId assetId;
        auto probeRes = media::MediaProbe::probe(wavFilePath.toStdString());
        if (probeRes.ok()) {
            auto asset = probeRes.unwrap();
            mediaLibrary_.addAsset(asset);
            assetId = asset->id();
            refresh();
        }

        // 5. Add clip with real asset reference
        core::TimelineTime insertTime = engine_.playback().currentTime();
        editor::Clip clip(
            core::ClipId::generate(),
            editor::ClipType::Audio,
            "Giọng đọc AI: " + text.left(16).toStdString() + "...",
            insertTime,
            core::TimelineTime::fromSeconds(durationSec)
        );
        clip.setParam("audio.ttsText", text.toStdString());
        clip.setParam("audio.ttsVoice", vId);
        clip.setParam("audio.speed", rateMultiplier);
        if (!assetId.str().empty()) {
            clip.setMediaId(assetId);
        }

        engine_.addClip(audioTrackId, std::move(clip));
        engine_.project().setDirty(true);
        engine_.notifyTimelineChanged();
    });

    ttsLayout->addWidget(genTtsBtn);
    cLayout->addWidget(ttsCard);

    cLayout->addStretch();
    scroll->setWidget(container);
    layout->addWidget(scroll, 1);

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
        UiIcon icon;
    };

    std::vector<GraphicItem> items = {
        {"Hình chữ nhật", "rectangle", UiIcon::ShapeRect},
        {"Hình tròn", "circle", UiIcon::ShapeCircle},
        {"Ngôi sao", "star", UiIcon::ShapeStar},
        {"Mũi tên", "arrow", UiIcon::ShapeArrow},
        {"Huy hiệu", "badge", UiIcon::Target},
        {"Khung viền", "frame", UiIcon::ShapeRect},
        {"Biểu đồ", "chart", UiIcon::Chart},
        {"Toàn màn", "fullscreen", UiIcon::Fullscreen},
        {"Hiệu ứng", "effect", UiIcon::Effects},
        {"Âm thanh", "audio", UiIcon::Audio},
        {"Văn bản", "text", UiIcon::Text},
        {"Cài đặt", "settings", UiIcon::Settings}
    };

    int row = 0, col = 0;
    for (const auto& it : items) {
        auto* btn = new QPushButton(gridContainer);
        btn->setIcon(UiIcons::get(it.icon, QColor("#38bdf8"), 20));
        btn->setIconSize(QSize(20, 20));
        btn->setText(it.name);
        btn->setFixedHeight(50);
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: #141417;
                border: 1px solid #27272a;
                border-radius: 6px;
                font-size: 11px;
                color: #f4f4f5;
                text-align: center;
                padding-left: 4px;
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

    auto* titleLabel = new QLabel("Hiệu ứng Video (Video Effects)", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    struct EffectItem {
        QString name;
        QString category;
        UiIcon icon;
    };

    std::vector<EffectItem> effects = {
        {"Làm mờ (Gaussian Blur)", "Bộ lọc Filter", UiIcon::Contrast},
        {"Chỉnh màu (Color Grading)", "Chỉnh màu", UiIcon::Adjustment},
        {"Tối góc (Vignette)", "Ống kính", UiIcon::Sun},
        {"Đen trắng (Monochrome)", "Chỉnh màu", UiIcon::Contrast},
        {"Đảo màu (Invert)", "Phong cách", UiIcon::Effects},
        {"Phóng to (Zoom In)", "Biến đổi", UiIcon::ZoomIn},
        {"Thu nhỏ (Zoom Out)", "Biến đổi", UiIcon::ZoomOut},
        {"Độ nét (Sharpen)", "Bộ lọc Filter", UiIcon::Sliders}
    };

    for (const auto& eff : effects) {
        auto* card = new QWidget(view);
        auto* cLayout = new QHBoxLayout(card);
        cLayout->setContentsMargins(10, 8, 10, 8);

        auto* ic = new QLabel(card);
        ic->setPixmap(UiIcons::getPixmap(eff.icon, QColor("#38bdf8"), 18));
        ic->setFixedSize(22, 22);
        cLayout->addWidget(ic);

        auto* lbl = new QLabel(QString("<b>%1</b><br><span style='color:#71717a; font-size:11px;'>%2</span>")
            .arg(eff.name).arg(eff.category), card);
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

QWidget* AssetsPanel::createTransitionsView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Hiệu ứng chuyển cảnh (Transitions)", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* durBox = new QGroupBox("Thời lượng chuyển cảnh", view);
    auto* durLayout = new QHBoxLayout(durBox);
    auto* durLabel = new QLabel("0.5s", durBox);
    durLabel->setStyleSheet("color: #38bdf8; font-weight: 600; min-width: 36px;");
    auto* durSlider = new QSlider(Qt::Horizontal, durBox);
    durSlider->setRange(2, 20); // 0.2s to 2.0s
    durSlider->setValue(5);
    connect(durSlider, &QSlider::valueChanged, [durLabel](int v) {
        durLabel->setText(QString("%1s").arg(v / 10.0, 0, 'f', 1));
    });
    durLayout->addWidget(durSlider);
    durLayout->addWidget(durLabel);
    layout->addWidget(durBox);

    auto* scroll = new QScrollArea(view);
    scroll->setWidgetResizable(true);
    auto* container = new QWidget(scroll);
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(0, 0, 0, 0);
    cLayout->setSpacing(6);

    struct TransitionItem {
        QString name;
        QString desc;
        UiIcon icon;
    };

    std::vector<TransitionItem> transList = {
        {"Cross Dissolve", "Hòa tan mờ dần giữa 2 cảnh", UiIcon::Transitions},
        {"Fade to Black", "Mờ dần sang nền đen rồi chuyển cảnh", UiIcon::Contrast},
        {"Fade to White", "Mờ lóa sáng sang trắng rồi hiện cảnh mới", UiIcon::Sun},
        {"Slide Left", "Trượt mượt mà sang bên trái", UiIcon::StepForward},
        {"Slide Right", "Trượt mượt mà sang bên phải", UiIcon::StepBack},
        {"Slide Up", "Trượt cảnh mới từ dưới lên trên", UiIcon::Transitions},
        {"Slide Down", "Trượt cảnh mới từ trên xuống dưới", UiIcon::Transitions},
        {"Zoom In", "Phóng to đột phá vào cảnh tiếp theo", UiIcon::ZoomIn},
        {"Zoom Out", "Thu nhỏ lùi dần chuyển cảnh", UiIcon::ZoomOut},
        {"Wipe", "Quét chuyển cảnh ngang sắc nét", UiIcon::Sliders}
    };

    for (const auto& tr : transList) {
        auto* card = new QWidget(container);
        auto* rowLayout = new QHBoxLayout(card);
        rowLayout->setContentsMargins(8, 8, 8, 8);

        auto* icLabel = new QLabel(card);
        icLabel->setPixmap(UiIcons::getPixmap(tr.icon, QColor("#38bdf8"), 20));
        rowLayout->addWidget(icLabel);

        auto* infoLabel = new QLabel(QString("<b>%1</b><br><span style='color:#71717a; font-size:11px;'>%2</span>")
            .arg(tr.name).arg(tr.desc), card);
        infoLabel->setTextFormat(Qt::RichText);
        rowLayout->addWidget(infoLabel, 1);

        auto* applyBtn = new QPushButton("Áp dụng", card);
        applyBtn->setFixedSize(68, 28);
        connect(applyBtn, &QPushButton::clicked, [this, tr, durSlider]() {
            double dur = durSlider->value() / 10.0;
            onApplyTransitionPreset(tr.name, dur);
        });
        rowLayout->addWidget(applyBtn);

        card->setStyleSheet("QWidget { background: #141417; border: 1px solid #27272a; border-radius: 6px; }");
        cLayout->addWidget(card);
    }
    cLayout->addStretch();
    scroll->setWidget(container);
    layout->addWidget(scroll, 1);

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

    auto* autoBtn = new QPushButton("Tạo phụ đề tự động (AI Auto-Caption)", view);
    autoBtn->setIcon(UiIcons::get(UiIcon::Captions, QColor("#ffffff"), 18));
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

    auto* importSrtBtn = new QPushButton("Nhập tệp phụ đề .SRT / .VTT", view);
    importSrtBtn->setIcon(UiIcons::get(UiIcon::Text, QColor("#f4f4f5"), 16));
    importSrtBtn->setFixedHeight(34);
    connect(importSrtBtn, &QPushButton::clicked, this, &AssetsPanel::onImportSrtClicked);
    layout->addWidget(importSrtBtn);

    layout->addStretch();

    return view;
}

QWidget* AssetsPanel::createAdjustmentView() {
    auto* view = new QWidget(this);
    auto* layout = new QVBoxLayout(view);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* titleLabel = new QLabel("Lớp điều chỉnh & Chỉnh màu (Adjustment)", view);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 14px; color: #f4f4f5;");
    layout->addWidget(titleLabel);

    auto* scroll = new QScrollArea(view);
    scroll->setWidgetResizable(true);
    auto* container = new QWidget(scroll);
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(0, 0, 0, 0);
    cLayout->setSpacing(8);

    auto makeSliderRow = [container](const QString& title, int min, int max, int def) -> std::pair<QSlider*, QLabel*> {
        auto* box = new QWidget(container);
        auto* bLayout = new QVBoxLayout(box);
        bLayout->setContentsMargins(8, 6, 8, 6);

        auto* topRow = new QHBoxLayout();
        auto* lbl = new QLabel(title, box);
        lbl->setStyleSheet("color: #f4f4f5; font-size: 12px; font-weight: 500;");
        auto* valLbl = new QLabel(QString::number(def), box);
        valLbl->setStyleSheet("color: #38bdf8; font-weight: 600; font-size: 12px;");
        topRow->addWidget(lbl);
        topRow->addStretch();
        topRow->addWidget(valLbl);
        bLayout->addLayout(topRow);

        auto* slider = new QSlider(Qt::Horizontal, box);
        slider->setRange(min, max);
        slider->setValue(def);
        QObject::connect(slider, &QSlider::valueChanged, [valLbl](int v) {
            valLbl->setText(QString::number(v));
        });
        bLayout->addWidget(slider);
        box->setStyleSheet("QWidget { background: #141417; border: 1px solid #27272a; border-radius: 6px; }");
        return {slider, valLbl};
    };

    auto [brightSlider, brightLbl] = makeSliderRow("Độ sáng (Brightness)", -100, 100, 0);
    auto [contrastSlider, contrastLbl] = makeSliderRow("Độ tương phản (Contrast)", -100, 100, 0);
    auto [satSlider, satLbl] = makeSliderRow("Độ bão hòa màu (Saturation)", -100, 100, 0);
    auto [tempSlider, tempLbl] = makeSliderRow("Nhiệt độ màu (Temperature)", -100, 100, 0);
    auto [tintSlider, tintLbl] = makeSliderRow("Sắc thái màu (Tint)", -100, 100, 0);

    cLayout->addWidget(brightSlider->parentWidget());
    cLayout->addWidget(contrastSlider->parentWidget());
    cLayout->addWidget(satSlider->parentWidget());
    cLayout->addWidget(tempSlider->parentWidget());
    cLayout->addWidget(tintSlider->parentWidget());

    auto* applyBtn = new QPushButton("Áp dụng cho Clip đang chọn", container);
    applyBtn->setFixedHeight(36);
    applyBtn->setStyleSheet(R"(
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
    connect(applyBtn, &QPushButton::clicked, [this, brightSlider, contrastSlider, satSlider, tempSlider, tintSlider]() {
        onApplyAdjustment(
            brightSlider->value(),
            contrastSlider->value(),
            satSlider->value(),
            tempSlider->value(),
            tintSlider->value()
        );
    });
    cLayout->addWidget(applyBtn);

    auto* resetBtn = new QPushButton("Đặt lại tất cả về 0", container);
    resetBtn->setFixedHeight(30);
    connect(resetBtn, &QPushButton::clicked, [=]() {
        brightSlider->setValue(0);
        contrastSlider->setValue(0);
        satSlider->setValue(0);
        tempSlider->setValue(0);
        tintSlider->setValue(0);
    });
    cLayout->addWidget(resetBtn);

    auto* addLayerBtn = new QPushButton("Thêm lớp Điều chỉnh vào Timeline", container);
    addLayerBtn->setIcon(UiIcons::get(UiIcon::Plus, QColor("#38bdf8"), 16));
    addLayerBtn->setIconSize(QSize(16, 16));
    addLayerBtn->setFixedHeight(34);
    connect(addLayerBtn, &QPushButton::clicked, this, &AssetsPanel::onAddAdjustmentLayer);
    cLayout->addWidget(addLayerBtn);

    cLayout->addStretch();
    scroll->setWidget(container);
    layout->addWidget(scroll, 1);

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
        UiIcon iconType = (asset->type() == media::MediaType::Video) ? UiIcon::Media :
                          (asset->type() == media::MediaType::Audio) ? UiIcon::Audio : UiIcon::Media;
        item->setIcon(UiIcons::get(iconType, QColor("#38bdf8"), 24));

        QString label = QString("%1\n<span style='color:#71717a;'>%2x%3 • %4</span>")
            .arg(QString::fromStdString(asset->fileName()))
            .arg(asset->width())
            .arg(asset->height())
            .arg(QString::fromStdString(durStr));

        item->setText(label);
        item->setData(Qt::UserRole, QString::fromStdString(asset->id().str()));
    }
}

void AssetsPanel::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void AssetsPanel::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void AssetsPanel::dropEvent(QDropEvent* event) {
    const QMimeData* mime = event->mimeData();
    if (!mime || !mime->hasUrls()) {
        event->ignore();
        return;
    }

    bool addedAny = false;
    for (const QUrl& url : mime->urls()) {
        if (!url.isLocalFile()) continue;
        QString localPath = url.toLocalFile();
        if (localPath.isEmpty()) continue;

        auto probeRes = media::MediaProbe::probe(localPath.toStdString());
        if (probeRes.ok()) {
            auto asset = probeRes.unwrap();
            mediaLibrary_.addAsset(asset);
            addedAny = true;

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

    if (addedAny) {
        refresh();
        event->acceptProposedAction();
    } else {
        event->ignore();
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
    if (sel.empty()) return;

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
    engine_.notifyProjectChanged();
}

void AssetsPanel::onApplyTransitionPreset(const QString& name, double durationSec) {
    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    for (const auto& cid : sel) {
        auto* clip = tl->findClip(cid);
        if (clip) {
            clip->setParam("transition.type", name.toStdString());
            clip->setParam("transition.duration", durationSec);
        }
    }
    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void AssetsPanel::onApplyAdjustment(double brightness, double contrast, double saturation, double temperature, double tint) {
    const auto& sel = engine_.selectedClips();
    if (sel.empty()) return;

    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    for (const auto& cid : sel) {
        auto* clip = tl->findClip(cid);
        if (clip) {
            clip->setParam("adjustment.brightness", brightness);
            clip->setParam("adjustment.contrast", contrast);
            clip->setParam("adjustment.saturation", saturation);
            clip->setParam("adjustment.temperature", temperature);
            clip->setParam("adjustment.tint", tint);
        }
    }
    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void AssetsPanel::onAddAdjustmentLayer() {
    auto* tl = engine_.activeTimeline();
    if (!tl) return;

    core::TimelineTime insertTime = engine_.playback().currentTime();
    editor::Clip clip(
        core::ClipId::generate(),
        editor::ClipType::Graphic,
        "Lớp điều chỉnh (Adjustment)",
        insertTime,
        core::TimelineTime::fromSeconds(5.0)
    );
    clip.setParam("isAdjustmentLayer", true);
    clip.setParam("adjustment.brightness", 0.0);
    clip.setParam("adjustment.contrast", 0.0);
    clip.setParam("adjustment.saturation", 0.0);

    engine_.addClip(tl->mainTrack().id(), std::move(clip));
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
    if (result.cues.empty()) return;

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
    engine_.project().setDirty(true);
    engine_.notifyTimelineChanged();
}

void AssetsPanel::onAutoTranscribeClicked() {
    auto* tl = engine_.activeTimeline();
    if (!tl || tl->mainTrack().clips().empty()) return;

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
    engine_.project().setDirty(true);
    engine_.notifyTimelineChanged();
}

void AssetsPanel::onCanvasAspectChanged(int width, int height) {
    engine_.project().settings().canvasSize = {width, height};
    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

void AssetsPanel::onCanvasBgColorChanged(const QColor& color) {
    engine_.project().settings().background.color = color.name().toStdString();
    engine_.project().setDirty(true);
    engine_.notifyProjectChanged();
}

} // namespace catchim::ui
#endif
