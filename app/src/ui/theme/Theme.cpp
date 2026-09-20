#include "Theme.h"

#if defined(HAVE_QT6)
#include <QApplication>

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
        qApp->setStyleSheet(buildGlobalStyleSheet());
    }
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
        QWidget {
            background-color: %1;
            color: %2;
            font-family: 'Inter', sans-serif;
            font-size: 13px;
            outline: none;
            border: none;
        }

        /* Panels */
        .Panel {
            background-color: %3;
            border: 1px solid %4;
            border-radius: 6px;
        }

        /* Buttons */
        QPushButton {
            background-color: transparent;
            color: %2;
            border: 1px solid transparent;
            border-radius: 4px;
            padding: 4px 8px;
        }
        QPushButton:hover {
            background-color: %5;
        }
        QPushButton:pressed {
            background-color: %6;
        }

        /* Splitters */
        QSplitter::handle {
            background-color: %1;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: %4;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: %7;
        }
        QScrollBar:horizontal {
            background: transparent;
            height: 8px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: %4;
            min-width: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: %7;
        }
        QScrollBar::add-line, QScrollBar::sub-line {
            width: 0px;
            height: 0px;
        }
    )")
    .arg(palette_.background.name())
    .arg(palette_.textPrimary.name())
    .arg(palette_.panelBackground.name())
    .arg(palette_.panelBorder.name())
    .arg(palette_.hover.name())
    .arg(palette_.secondary.name())
    .arg(palette_.textSecondary.name());

    return qss;
}

} // namespace catchim::ui
#endif
