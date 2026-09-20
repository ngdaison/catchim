#include "EditorHeader.h"

#if defined(HAVE_QT6)
#include "ui/theme/Theme.h"
#include <QMenu>
#include <QMessageBox>

namespace catchim::ui {

EditorHeader::EditorHeader(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setFixedHeight(Metrics::headerHeight);
    setStyleSheet("background-color: #0c0c0e; border-bottom: 1px solid #27272a;");
    setupUi();
    refresh();
}

void EditorHeader::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 12, 4);
    layout->setSpacing(8);

    // Left side: Logo & Project Name
    auto* logoButton = new QPushButton("✦", this);
    logoButton->setFixedSize(32, 32);
    logoButton->setStyleSheet(R"(
        QPushButton {
            font-size: 16px;
            color: #38bdf8;
            background: #18181b;
            border: 1px solid #27272a;
            border-radius: 6px;
        }
        QPushButton:hover {
            background: #27272a;
            border-color: #38bdf8;
        }
    )");

    auto* logoMenu = new QMenu(logoButton);
    logoMenu->addAction("📝 Tạo dự án mới", [this]() {
        engine_.newProject("Dự án mới");
    });
    logoMenu->addSeparator();
    logoMenu->addAction("⌨ Phím tắt (Shortcuts)", [this]() {
        QMessageBox::information(this, "Phím tắt Catchim",
            "• Space: Phát / Tạm dừng\n"
            "• S: Cắt clip tại đầu đọc\n"
            "• Ctrl+D: Nhân bản clip\n"
            "• Delete / Backspace: Xóa clip đang chọn\n"
            "• N: Bật/tắt hít nam châm (Snapping)\n"
            "• Ctrl+Z: Hoàn tác (Undo)\n"
            "• Ctrl+Y: Làm lại (Redo)\n"
            "• Home / End: Về đầu / Về cuối video");
    });
    logoButton->setMenu(logoMenu);

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setFixedHeight(32);
    nameEdit_->setStyleSheet(R"(
        QLineEdit {
            background: transparent;
            color: #f4f4f5;
            font-weight: 600;
            font-size: 13px;
            padding: 0px 8px;
            border-radius: 6px;
            border: 1px solid transparent;
        }
        QLineEdit:hover {
            background: #18181b;
            border: 1px solid #27272a;
        }
        QLineEdit:focus {
            background: #18181b;
            border: 1px solid #38bdf8;
        }
    )");
    connect(nameEdit_, &QLineEdit::editingFinished, this, &EditorHeader::onNameEditingFinished);

    layout->addWidget(logoButton);
    layout->addWidget(nameEdit_);

    // Undo / Redo Quick Buttons
    auto* undoBtn = new QPushButton("↩", this);
    undoBtn->setFixedSize(30, 30);
    undoBtn->setToolTip("Hoàn tác (Ctrl+Z)");
    connect(undoBtn, &QPushButton::clicked, [this]() {
        engine_.undo();
    });

    auto* redoBtn = new QPushButton("↪", this);
    redoBtn->setFixedSize(30, 30);
    redoBtn->setToolTip("Làm lại (Ctrl+Y)");
    connect(redoBtn, &QPushButton::clicked, [this]() {
        engine_.redo();
    });

    layout->addWidget(undoBtn);
    layout->addWidget(redoBtn);

    layout->addStretch();

    // Right side: Export Button & Theme toggle
    exportButton_ = new QPushButton("🚀 Xuất video", this);
    exportButton_->setFixedHeight(32);
    exportButton_->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0284c7, stop:1 #38bdf8);
            color: #ffffff;
            font-weight: 700;
            font-size: 12px;
            padding: 0px 16px;
            border-radius: 6px;
            border: none;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0369a1, stop:1 #0284c7);
        }
    )");
    connect(exportButton_, &QPushButton::clicked, this, &EditorHeader::exportClicked);

    themeButton_ = new QPushButton("◐", this);
    themeButton_->setFixedSize(32, 32);
    themeButton_->setToolTip("Chuyển chế độ Sáng / Tối (Theme)");
    themeButton_->setStyleSheet(R"(
        QPushButton {
            font-size: 14px;
            color: #a1a1aa;
            background: #18181b;
            border: 1px solid #27272a;
            border-radius: 6px;
        }
        QPushButton:hover {
            background: #27272a;
            color: #f4f4f5;
        }
    )");
    connect(themeButton_, &QPushButton::clicked, this, &EditorHeader::themeToggleClicked);

    layout->addWidget(exportButton_);
    layout->addWidget(themeButton_);
}

void EditorHeader::refresh() {
    nameEdit_->setText(QString::fromStdString(engine_.project().name()));
}

void EditorHeader::onNameEditingFinished() {
    std::string newName = nameEdit_->text().trimmed().toStdString();
    if (!newName.empty()) {
        engine_.renameProject(newName);
    } else {
        refresh();
    }
}

} // namespace catchim::ui
#endif
