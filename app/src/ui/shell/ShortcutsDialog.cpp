#include "ShortcutsDialog.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>

namespace catchim::ui {

ShortcutsDialog::ShortcutsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Phím tắt Catchim");
    setModal(true);
    setFixedSize(580, 520);
    setStyleSheet(R"(
        QDialog {
            background-color: #0e0e11;
            border: 1px solid #27272a;
            border-radius: 10px;
        }
    )");
    setupUi();
}

void ShortcutsDialog::setupUi() {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 16, 20, 16);
    rootLayout->setSpacing(12);

    // Header
    auto* headerLayout = new QHBoxLayout();
    auto* iconLabel = new QLabel(this);
    iconLabel->setPixmap(UiIcons::getPixmap(UiIcon::Keyboard, QColor("#38bdf8"), 20));
    headerLayout->addWidget(iconLabel);

    auto* titleLabel = new QLabel("Phím tắt ứng dụng (Keyboard Shortcuts)", this);
    titleLabel->setStyleSheet("font-size: 15px; font-weight: 700; color: #f4f4f5;");
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();

    auto* closeBtn = new QPushButton("✕", this);
    closeBtn->setFixedSize(28, 28);
    closeBtn->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #a1a1aa;
            font-size: 14px;
            font-weight: 600;
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover {
            background: #27272a;
            color: #f4f4f5;
        }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    headerLayout->addWidget(closeBtn);
    rootLayout->addLayout(headerLayout);

    // Separator line
    auto* sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #27272a;");
    rootLayout->addWidget(sep);

    // Scroll Area
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    auto* container = new QWidget(scroll);
    auto* cLayout = new QVBoxLayout(container);
    cLayout->setContentsMargins(0, 0, 8, 0);
    cLayout->setSpacing(16);

    struct ShortcutDef {
        QString action;
        QString keys;
    };

    struct CategoryDef {
        QString name;
        std::vector<ShortcutDef> shortcuts;
    };

    std::vector<CategoryDef> categories = {
        {
            "Phát lại & Điều hướng (Playback & Navigation)",
            {
                {"Phát / Tạm dừng video", "Space"},
                {"Về đầu dòng thời gian", "Home"},
                {"Đến cuối dòng thời gian", "End"},
                {"Lùi 1 khung hình", "◄ (Trái)"},
                {"Tiến 1 khung hình", "► (Phải)"},
                {"Nhảy lùi 5 giây", "Shift + ◄"},
                {"Nhảy tiến 5 giây", "Shift + ►"}
            }
        },
        {
            "Chỉnh sửa dòng thời gian (Timeline Editing)",
            {
                {"Cắt phần tử tại đầu đọc (Split)", "S"},
                {"Cắt bỏ đoạn bên trái (Split Left)", "Q"},
                {"Cắt bỏ đoạn bên phải (Split Right)", "W"},
                {"Nhân bản phần tử (Duplicate)", "Ctrl + D"},
                {"Xóa phần tử đang chọn (Delete)", "Delete / Backspace"},
                {"Đánh dấu mốc thời gian (Marker)", "M"},
                {"Bật/tắt tự động hít nam châm (Snapping)", "N"},
                {"Bật/tắt chế độ Ripple Editing", "R"}
            }
        },
        {
            "Lịch sử & Dự án (History & Project)",
            {
                {"Hoàn tác thay đổi (Undo)", "Ctrl + Z"},
                {"Làm lại thay đổi (Redo)", "Ctrl + Y"},
                {"Chọn tất cả phần tử (Select All)", "Ctrl + A"},
                {"Bỏ chọn tất cả phần tử", "Escape"},
                {"Lưu dự án", "Ctrl + S"}
            }
        },
        {
            "Khung xem trước & Xuất bản (Preview & Export)",
            {
                {"Bật/tắt khung an toàn (Safe Zones)", "T"},
                {"Chế độ toàn màn hình (Fullscreen)", "F11"},
                {"Hộp thoại xuất video hoàn chỉnh", "Ctrl + E"}
            }
        }
    };

    for (const auto& cat : categories) {
        auto* catHeader = new QLabel(cat.name, container);
        catHeader->setStyleSheet("font-size: 11px; font-weight: 700; color: #38bdf8; text-transform: uppercase; letter-spacing: 0.5px; padding-top: 4px;");
        cLayout->addWidget(catHeader);

        auto* catBox = new QWidget(container);
        auto* boxLayout = new QVBoxLayout(catBox);
        boxLayout->setContentsMargins(10, 8, 10, 8);
        boxLayout->setSpacing(8);
        catBox->setStyleSheet("background-color: #141417; border: 1px solid #27272a; border-radius: 6px;");

        for (const auto& item : cat.shortcuts) {
            auto* row = new QHBoxLayout();
            row->setContentsMargins(0, 0, 0, 0);

            auto* descLabel = new QLabel(item.action, catBox);
            descLabel->setStyleSheet("font-size: 12px; color: #e4e4e7; font-weight: 500;");
            row->addWidget(descLabel);

            row->addStretch();

            // Render key badge
            auto* kbdLabel = new QLabel(item.keys, catBox);
            kbdLabel->setStyleSheet(R"(
                QLabel {
                    background-color: #27272a;
                    color: #f4f4f5;
                    font-family: 'Cascadia Code', Consolas, monospace;
                    font-size: 11px;
                    font-weight: 600;
                    padding: 3px 8px;
                    border: 1px solid #3f3f46;
                    border-radius: 4px;
                }
            )");
            row->addWidget(kbdLabel);

            boxLayout->addLayout(row);
        }

        cLayout->addWidget(catBox);
    }

    cLayout->addStretch();
    scroll->setWidget(container);
    rootLayout->addWidget(scroll, 1);

    // Footer button
    auto* footerLayout = new QHBoxLayout();
    footerLayout->addStretch();
    auto* doneBtn = new QPushButton("Đóng", this);
    doneBtn->setFixedSize(80, 32);
    doneBtn->setStyleSheet(R"(
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
    connect(doneBtn, &QPushButton::clicked, this, &QDialog::accept);
    footerLayout->addWidget(doneBtn);
    rootLayout->addLayout(footerLayout);
}

} // namespace catchim::ui
#endif
