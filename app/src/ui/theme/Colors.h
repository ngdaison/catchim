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
    p.background = QColor("#09090b");       // zinc-950
    p.panelBackground = QColor("#121215");  // zinc-900 surface
    p.textPrimary = QColor("#f4f4f5");      // zinc-100
    p.textSecondary = QColor("#a1a1aa");    // zinc-400
    p.primaryAccent = QColor("#38bdf8");    // sky-400
    p.secondary = QColor("#27272a");        // zinc-800
    p.border = QColor("#27272a");           // zinc-800
    p.panelBorder = QColor("#27272a");      // zinc-800
    p.hover = QColor("#1e1e24");            // zinc-800 hover
    p.selected = QColor("#0284c7");         // sky-600
    p.destructive = QColor("#ef4444");      // red-500

    // Clip colors matching web timeline
    p.clipVideo = QColor("#1e293b");        // slate-800
    p.clipAudio = QColor("#581c87");        // purple-900
    p.clipText = QColor("#065f46");         // emerald-900
    p.clipGraphic = QColor("#831843");      // pink-900
    p.clipEffect = QColor("#1e3a8a");       // blue-900
    p.clipBookmark = QColor("#38bdf8");     // sky-400
    p.waveformColor = QColor(255, 255, 255, 180);
    return p;
}

inline Palette getLightPalette() {
    Palette p;
    p.background = QColor("#f8fafc");
    p.panelBackground = QColor("#ffffff");
    p.textPrimary = QColor("#0f172a");
    p.textSecondary = QColor("#64748b");
    p.primaryAccent = QColor("#0284c7");
    p.secondary = QColor("#f1f5f9");
    p.border = QColor("#e2e8f0");
    p.panelBorder = QColor("#cbd5e1");
    p.hover = QColor("#f1f5f9");
    p.selected = QColor("#e0f2fe");
    p.destructive = QColor("#ef4444");

    p.clipVideo = QColor("#94a3b8");
    p.clipAudio = QColor("#c084fc");
    p.clipText = QColor("#6ee7b7");
    p.clipGraphic = QColor("#f472b6");
    p.clipEffect = QColor("#60a5fa");
    p.clipBookmark = QColor("#0284c7");
    p.waveformColor = QColor(0, 0, 0, 180);
    return p;
}
#endif

} // namespace catchim::ui
