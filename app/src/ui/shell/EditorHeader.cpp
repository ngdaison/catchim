#include "EditorHeader.h"

#if defined(HAVE_QT6)
#include "ui/icons/UiIcons.h"
#include "ui/theme/Theme.h"
#include "ShortcutsDialog.h"
#include <QMenu>

namespace catchim::ui {

EditorHeader::EditorHeader(editor::EditorEngine& engine, QWidget* parent)
    : QWidget(parent)
    , engine_(engine)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(Metrics::headerHeight);
    setupUi();
    refresh();
}

void EditorHeader::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 12, 4);
    layout->setSpacing(8);

    // Left side: Logo & Project Name
    logoButton_ = new QPushButton(this);
    logoButton_->setFixedSize(32, 32);
    logoButton_->setIconSize(QSize(18, 18));
    logoButton_->setStyleSheet("QPushButton::menu-indicator { image: none; width: 0px; }");

    auto* logoMenu = new QMenu(logoButton_);
    logoMenu->addAction("Tạo dự án mới", [this]() {
        engine_.newProject("Dự án mới");
    });
    logoMenu->addSeparator();
    logoMenu->addAction("Phím tắt (Shortcuts)", [this]() {
        ShortcutsDialog dlg(this);
        dlg.exec();
    });
    logoButton_->setMenu(logoMenu);

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setFixedHeight(32);
    connect(nameEdit_, &QLineEdit::editingFinished, this, &EditorHeader::onNameEditingFinished);

    layout->addWidget(logoButton_);
    layout->addWidget(nameEdit_);

    // Undo / Redo Quick Buttons
    undoBtn_ = new QPushButton(this);
    undoBtn_->setFixedSize(30, 30);
    undoBtn_->setIconSize(QSize(16, 16));
    undoBtn_->setToolTip("Hoàn tác (Ctrl+Z)");
    connect(undoBtn_, &QPushButton::clicked, [this]() {
        engine_.undo();
    });

    redoBtn_ = new QPushButton(this);
    redoBtn_->setFixedSize(30, 30);
    redoBtn_->setIconSize(QSize(16, 16));
    redoBtn_->setToolTip("Làm lại (Ctrl+Y)");
    connect(redoBtn_, &QPushButton::clicked, [this]() {
        engine_.redo();
    });

    layout->addWidget(undoBtn_);
    layout->addWidget(redoBtn_);

    layout->addStretch();

    // Right side: Export Button & Theme toggle
    exportButton_ = new QPushButton("Xuất video", this);
    exportButton_->setIcon(UiIcons::get(UiIcon::Export, QColor("#ffffff"), 16));
    exportButton_->setIconSize(QSize(16, 16));
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

    themeButton_ = new QPushButton(this);
    themeButton_->setFixedSize(32, 32);
    themeButton_->setIconSize(QSize(16, 16));
    connect(themeButton_, &QPushButton::clicked, this, &EditorHeader::themeToggleClicked);

    layout->addWidget(exportButton_);
    layout->addWidget(themeButton_);
}

void EditorHeader::refresh() {
    nameEdit_->setText(QString::fromStdString(engine_.project().name()));
    const auto& pal = Theme::instance().palette();
    bool isDark = (Theme::instance().mode() == ThemeMode::Dark);
    if (themeButton_) {
        themeButton_->setIcon(UiIcons::get(UiIcon::Sun, pal.textSecondary, 16));
        themeButton_->setToolTip(isDark ? "Chuyển sang giao diện sáng" : "Chuyển sang giao diện tối");
    }
    if (undoBtn_) {
        undoBtn_->setIcon(UiIcons::get(UiIcon::Undo, pal.textPrimary, 16));
    }
    if (redoBtn_) {
        redoBtn_->setIcon(UiIcons::get(UiIcon::Redo, pal.textPrimary, 16));
    }
    if (logoButton_) {
        logoButton_->setIcon(UiIcons::get(UiIcon::Media, pal.primaryAccent, 18));
    }
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
