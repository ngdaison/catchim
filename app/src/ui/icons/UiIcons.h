#pragma once

#if defined(HAVE_QT6)
#include <QIcon>
#include <QPixmap>
#include <QColor>
#include <QSize>

namespace catchim::ui {

enum class UiIcon {
    Media,
    Audio,
    Text,
    Stickers,
    Effects,
    Transitions,
    Captions,
    Settings,
    Split,
    Duplicate,
    Delete,
    Magnet,
    Play,
    Pause,
    StepBack,
    StepForward,
    SkipBack,
    SkipForward,
    ZoomIn,
    ZoomOut,
    Undo,
    Redo,
    Export,
    Plus,
    Eye,
    EyeOff,
    Volume,
    VolumeMute,
    Search
};

class UiIcons {
public:
    static QIcon get(UiIcon icon, const QColor& color = QColor("#f4f4f5"), int size = 20);
    static QPixmap getPixmap(UiIcon icon, const QColor& color = QColor("#f4f4f5"), int size = 20);
    static const char* getSvgContent(UiIcon icon);
};

} // namespace catchim::ui
#endif
