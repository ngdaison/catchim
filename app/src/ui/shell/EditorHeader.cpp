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
    setupUi();
    refresh();
}

void EditorHeader::setupUi() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 2, 12, 2);
    layout->setSpacing(8);

    // Left side: Logo & Project Name
    auto* logoButton = new QPushButton("✦", this);
    logoButton->setFixedSize(32, 32);
    logoButton->setStyleSheet(R"(
        QPushButton {
            font-size: 16px;
            color: #16A9F3;
            background: #1A1A1A;
            border: 1px solid #292929;
            border-radius: 4px;
        }
        QPushButton:hover {
            background: #242424;
        }
    )");

    auto* logoMenu = new QMenu(logoButton);
    logoMenu->addAction("Phím tắt (Shortcuts)", []() {});
    logoMenu->addSeparator();
    logoMenu->addAction("Thoát dự án", [this]() {
        engine_.newProject("New project");
    });
    logoButton->setMenu(logoMenu);

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setFixedHeight(32);
    nameEdit_->setStyleSheet(R"(
        QLineEdit {
            background: transparent;
            color: #DEDEDE;
            font-weight: 500;
            font-size: 14px;
            padding: 0px 8px;
            border-radius: 4px;
            border: 1px solid transparent;
        }
        QLineEdit:hover {
            background: #1A1A1A;
            border: 1px solid #292929;
        }
        QLineEdit:focus {
            background: #1A1A1A;
            border: 1px solid #16A9F3;
        }
    )");
    connect(nameEdit_, &QLineEdit::editingFinished, this, &EditorHeader::onNameEditingFinished);

    layout->addWidget(logoButton);
    layout->addWidget(nameEdit_);

    layout->addStretch();

    // Right side: Export Button & Theme toggle
    exportButton_ = new QPushButton("Xuất video", this);
    exportButton_->setFixedHeight(32);
    exportButton_->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2567EC, stop:1 #37B6F7);
            color: white;
            font-weight: 600;
            font-size: 13px;
            padding: 0px 16px;
            border-radius: 6px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2B73FF, stop:1 #4BC0FF);
        }
    )");
    connect(exportButton_, &QPushButton::clicked, this, &EditorHeader::exportClicked);

    themeButton_ = new QPushButton("◐", this);
    themeButton_->setFixedSize(32, 32);
    themeButton_->setStyleSheet(R"(
        QPushButton {
            font-size: 14px;
            color: #808080;
            border-radius: 4px;
        }
        QPushButton:hover {
            background: #242424;
            color: #DEDEDE;
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
