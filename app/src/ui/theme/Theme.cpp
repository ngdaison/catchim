#include "Theme.h"

#if defined(HAVE_QT6)
#include <QApplication>
#include <QPalette>

namespace catchim::ui {

Theme& Theme::instance() {
    static Theme s_instance;
    return s_instance;
}

Theme::Theme()
    : palette_(getDarkPalette())
{
}

void Theme::setTheme(ThemeMode mode) {
    mode_ = mode;
    palette_ = (mode == ThemeMode::Dark) ? getDarkPalette() : getLightPalette();
    if (qApp) {
        QPalette pal;
        pal.setColor(QPalette::Window, palette_.background);
        pal.setColor(QPalette::WindowText, palette_.textPrimary);
        pal.setColor(QPalette::Base, palette_.panelBackground);
        pal.setColor(QPalette::AlternateBase, palette_.secondary);
        pal.setColor(QPalette::ToolTipBase, palette_.panelBackground);
        pal.setColor(QPalette::ToolTipText, palette_.textPrimary);
        pal.setColor(QPalette::Text, palette_.textPrimary);
        pal.setColor(QPalette::Button, palette_.secondary);
        pal.setColor(QPalette::ButtonText, palette_.textPrimary);
        pal.setColor(QPalette::BrightText, palette_.destructive);
        pal.setColor(QPalette::Link, palette_.primaryAccent);
        pal.setColor(QPalette::Highlight, palette_.primaryAccent);
        pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
        qApp->setPalette(pal);
        qApp->setStyleSheet(buildGlobalStyleSheet());
    }
    for (const auto& [k, cb] : listeners_) {
        if (cb) cb(mode_);
    }
}

void Theme::addListener(const std::string& key, ThemeListener listener) {
    listeners_[key] = std::move(listener);
}

void Theme::removeListener(const std::string& key) {
    listeners_.erase(key);
}

QFont Theme::fontSans(int pointSize, int weight) const {
    QFont f("Inter", pointSize, weight);
    f.setStyleStrategy(QFont::PreferAntialias);
    return f;
}

QFont Theme::fontMono(int pointSize) const {
    QFont f("Cascadia Code", pointSize);
    f.setStyleHint(QFont::Monospace);
    return f;
}

QString Theme::buildGlobalStyleSheet() const {
    QString qss = QString(R"(
        /* Global Window & Central Widget */
        QMainWindow, QWidget#centralWidget {
            background-color: %1;
            color: %2;
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, 'Inter', Roboto, sans-serif;
            font-size: 13px;
        }

        QWidget {
            color: %2;
            outline: none;
        }

        QLabel {
            background-color: transparent;
            color: %2;
        }

        QLabel[class="SecondaryLabel"] {
            color: %5;
            font-size: 11px;
        }

        QLabel[class="SectionTitle"] {
            font-weight: 600;
            font-size: 14px;
            color: %2;
        }

        /* Panels & Frames */
        .Panel, QWidget[class="Panel"], QFrame#panelFrame, QWidget#assetsPanel, QWidget#previewPanel, QWidget#propertiesPanel, QWidget#timelinePanel {
            background-color: %3;
            border: 1px solid %4;
            border-radius: 8px;
        }

        /* Editor Header */
        #editorHeader {
            background-color: %3;
            border-bottom: 1px solid %4;
        }
        #editorHeader QPushButton {
            background-color: %6;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 0px;
        }
        #editorHeader QPushButton:hover {
            background-color: %7;
            border-color: %8;
        }
        #editorHeader QLineEdit {
            background: transparent;
            color: %2;
            font-weight: 600;
            font-size: 13px;
            padding: 0px 8px;
            border-radius: 6px;
            border: 1px solid transparent;
        }
        #editorHeader QLineEdit:hover {
            background: %6;
            border: 1px solid %4;
        }
        #editorHeader QLineEdit:focus {
            background: %6;
            border: 1px solid %8;
        }

        /* Left TabList in AssetsPanel */
        #tabList {
            background-color: %3;
            border: none;
            border-right: 1px solid %4;
            padding: 6px 0px;
            outline: none;
        }
        #tabList::item {
            height: 36px;
            width: 36px;
            border-radius: 6px;
            margin: 2px 4px;
        }
        #tabList::item:hover {
            background-color: %7;
        }
        #tabList::item:selected {
            background-color: %6;
        }

        /* Toolbars */
        #previewToolbar {
            background-color: %3;
            border-top: 1px solid %4;
        }
        #timelineToolbar {
            background-color: %3;
            border-bottom: 1px solid %4;
        }
        #previewToolbar QPushButton, #timelineToolbar QPushButton {
            background-color: %6;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 0px;
        }
        #previewToolbar QPushButton:hover, #timelineToolbar QPushButton:hover {
            background-color: %7;
            border-color: %8;
        }
        #previewToolbar QPushButton:checked, #timelineToolbar QPushButton:checked {
            background-color: %8;
            color: #ffffff;
            border-color: %8;
        }

        #sceneLabel {
            color: %2;
            font-weight: 600;
            font-size: 12px;
            padding: 4px 12px;
            background-color: %6;
            border: 1px solid %4;
            border-radius: 6px;
        }

        #timecodeLabel {
            font-family: 'Cascadia Code', monospace;
            color: %2;
            font-size: 12px;
            font-weight: 600;
            padding: 4px 8px;
            background-color: %6;
            border-radius: 4px;
            border: 1px solid %4;
        }

        /* Card Widgets */
        QWidget[class="cardWidget"] {
            background-color: %6;
            border: 1px solid %4;
            border-radius: 6px;
        }

        /* Import Button */
        #importBtn {
            background-color: %6;
            color: %2;
            border: 1px dashed %4;
            border-radius: 6px;
            font-weight: 600;
            font-size: 12px;
            padding: 0px 12px;
        }
        #importBtn:hover {
            background-color: %7;
            border-color: %8;
            color: %8;
        }

        /* Media List Widget */
        #mediaListWidget {
            background: transparent;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 4px;
        }
        #mediaListWidget::item {
            height: 52px;
            padding: 6px 10px;
            border-bottom: 1px solid %4;
            color: %2;
        }
        #mediaListWidget::item:hover {
            background-color: %7;
        }
        #mediaListWidget::item:selected {
            background-color: %8;
            color: #ffffff;
        }

        /* QSplitter */
        QSplitter {
            background-color: %1;
        }
        QSplitter::handle {
            background-color: %4;
        }
        QSplitter::handle:hover {
            background-color: %5;
        }

        /* Scroll Areas */
        QScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollArea > QWidget > QWidget {
            background-color: transparent;
        }
        #timelineScrollArea {
            border: none;
            background-color: %1;
        }

        /* Buttons */
        QPushButton {
            background-color: %6;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 6px 12px;
            font-weight: 500;
        }
        QPushButton:hover {
            background-color: %7;
            border-color: %8;
        }
        QPushButton:pressed {
            background-color: %6;
        }
        QPushButton:checked {
            background-color: %8;
            color: #ffffff;
            border-color: %8;
        }

        /* LineEdits, TextEdits, SpinBoxes */
        QLineEdit, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox {
            background-color: %6;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 4px 8px;
            selection-background-color: %8;
            selection-color: #ffffff;
        }
        QLineEdit:hover, QTextEdit:hover, QSpinBox:hover, QDoubleSpinBox:hover {
            border-color: %5;
        }
        QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: %8;
            background-color: %6;
        }

        /* SpinBox up/down buttons */
        QSpinBox::up-button, QDoubleSpinBox::up-button, QSpinBox::down-button, QDoubleSpinBox::down-button {
            background: transparent;
            border: none;
            width: 16px;
        }
        QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {
            image: none;
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 5px solid %5;
        }
        QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {
            image: none;
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid %5;
        }

        /* ComboBox */
        QComboBox {
            background-color: %6;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            padding: 4px 10px;
            min-height: 24px;
        }
        QComboBox:hover {
            border-color: %5;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 20px;
            border-left-width: 0px;
            border-top-right-radius: 6px;
            border-bottom-right-radius: 6px;
        }
        QComboBox::down-arrow {
            width: 0;
            height: 0;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 5px solid %5;
        }
        QComboBox QAbstractItemView {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 6px;
            selection-background-color: %8;
            selection-color: #ffffff;
            outline: none;
            padding: 4px;
        }

        /* GroupBox */
        QGroupBox {
            background-color: %3;
            color: %2;
            font-weight: 600;
            border: 1px solid %4;
            border-radius: 8px;
            margin-top: 18px;
            padding-top: 14px;
            padding-left: 10px;
            padding-right: 10px;
            padding-bottom: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 6px;
            background-color: %3;
            color: %2;
        }

        /* Tab Widgets & TabBar */
        QTabWidget::pane {
            border: 1px solid %4;
            background-color: %3;
            border-radius: 8px;
        }
        QTabBar::tab {
            background-color: %6;
            color: %5;
            padding: 8px 16px;
            margin-right: 2px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
        }
        QTabBar::tab:hover {
            background-color: %7;
            color: %2;
        }
        QTabBar::tab:selected {
            background-color: %3;
            color: %8;
            border-bottom: 2px solid %8;
            font-weight: 600;
        }

        /* List Widgets */
        QListWidget {
            background-color: transparent;
            border: none;
            outline: none;
        }
        QListWidget::item {
            border-radius: 6px;
            padding: 6px 10px;
            color: %2;
        }
        QListWidget::item:hover {
            background-color: %7;
        }
        QListWidget::item:selected {
            background-color: %8;
            color: #ffffff;
        }

        /* Sliders */
        QSlider::groove:horizontal {
            height: 4px;
            background: %4;
            border-radius: 2px;
        }
        QSlider::sub-page:horizontal {
            background: %8;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: %2;
            width: 14px;
            margin-top: -5px;
            margin-bottom: -5px;
            border-radius: 7px;
        }
        QSlider::handle:horizontal:hover {
            background: %8;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: %4;
            min-height: 24px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: %5;
        }
        QScrollBar::horizontal {
            background: transparent;
            height: 8px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: %4;
            min-width: 24px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: %5;
        }
        QScrollBar::add-line, QScrollBar::sub-line, QScrollBar::add-page, QScrollBar::sub-page {
            background: none;
            width: 0px;
            height: 0px;
        }

        /* Tooltips */
        QToolTip {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 4px;
            padding: 4px 8px;
            font-size: 11px;
        }

        /* Menu */
        QMenu {
            background-color: %3;
            color: %2;
            border: 1px solid %4;
            border-radius: 8px;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 20px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: %7;
            color: %8;
        }
        QMenu::separator {
            height: 1px;
            background-color: %4;
            margin: 4px 8px;
        }
    )")
    .arg(palette_.background.name())          // %1
    .arg(palette_.textPrimary.name())         // %2
    .arg(palette_.panelBackground.name())     // %3
    .arg(palette_.panelBorder.name())         // %4
    .arg(palette_.textSecondary.name())       // %5
    .arg(palette_.secondary.name())           // %6
    .arg(palette_.hover.name())               // %7
    .arg(palette_.primaryAccent.name());      // %8

    return qss;
}

} // namespace catchim::ui
#endif
