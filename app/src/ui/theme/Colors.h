#pragma once

#if defined(HAVE_QT6)
#include <QColor>
#endif

namespace catchim::ui {

#if defined(HAVE_QT6)
struct Palette {
    QColor background;
    QColor panelBackground;
    QColor textPrimary;
    QColor textSecondary;
    QColor primaryAccent;
    QColor secondary;
    QColor border;
    QColor panelBorder;
    QColor hover;
    QColor selected;
    QColor destructive;

    // Timeline clip colors
    QColor clipVideo;
    QColor clipAudio;
    QColor clipText;
    QColor clipGraphic;
    QColor clipEffect;
    QColor clipBookmark;
    QColor waveformColor;
};

inline Palette getDarkPalette() {
    Palette p;
    p.background = QColor("#0D0D0D");
    p.panelBackground = QColor("#1A1A1A");
    p.textPrimary = QColor("#DEDEDE");
    p.textSecondary = QColor("#808080");
    p.primaryAccent = QColor("#16A9F3");
    p.secondary = QColor("#00223D");
    p.border = QColor("#292929");
    p.panelBorder = QColor("#2E2E2E");
    p.hover = QColor("#242424");
    p.selected = QColor(255, 255, 255, 15);
    p.destructive = QColor("#EA1616");

    // Clip colors
    p.clipVideo = QColor("#232D41");
    p.clipAudio = QColor("#8F5DBA");
    p.clipText = QColor("#5DBAA0");
    p.clipGraphic = QColor("#BA5D7A");
    p.clipEffect = QColor("#5D93BA");
    p.clipBookmark = QColor("#009DFF");
    p.waveformColor = QColor(255, 255, 255, 178); // rgba(255, 255, 255, 0.7)
    return p;
}

inline Palette getLightPalette() {
    Palette p;
    p.background = QColor("#FFFFFF");
    p.panelBackground = QColor("#F8FAFC");
    p.textPrimary = QColor("#1C1C1C");
    p.textSecondary = QColor("#7F7F7F");
    p.primaryAccent = QColor("#16A9F3");
    p.secondary = QColor("#F0F8FF");
    p.border = QColor("#E8E8E8");
    p.panelBorder = QColor("#DEDEDE");
    p.hover = QColor("#F1F5F9");
    p.selected = QColor(0, 0, 0, 15);
    p.destructive = QColor("#EA1616");

    p.clipVideo = QColor("#CBD5E1");
    p.clipAudio = QColor("#8F5DBA");
    p.clipText = QColor("#5DBAA0");
    p.clipGraphic = QColor("#BA5D7A");
    p.clipEffect = QColor("#5D93BA");
    p.clipBookmark = QColor("#009DFF");
    p.waveformColor = QColor(255, 255, 255, 178);
    return p;
}
#endif

} // namespace catchim::ui
