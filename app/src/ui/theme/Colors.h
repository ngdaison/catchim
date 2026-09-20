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
    p.secondary = QColor("#18181b");        // zinc-900
    p.border = QColor("#27272a");           // zinc-800
    p.panelBorder = QColor("#27272a");      // zinc-800
    p.hover = QColor("#27272a");            // zinc-800 hover
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
    p.background = QColor("#f4f4f5");       // zinc-100
    p.panelBackground = QColor("#ffffff");  // white surface
    p.textPrimary = QColor("#09090b");      // zinc-950
    p.textSecondary = QColor("#71717a");    // zinc-500
    p.primaryAccent = QColor("#0284c7");    // sky-600
    p.secondary = QColor("#f4f4f5");        // zinc-100
    p.border = QColor("#e4e4e7");           // zinc-200
    p.panelBorder = QColor("#e4e4e7");      // zinc-200
    p.hover = QColor("#e4e4e7");            // zinc-200 hover
    p.selected = QColor("#e0f2fe");         // sky-100
    p.destructive = QColor("#ef4444");      // red-500

    p.clipVideo = QColor("#2563eb");        // blue-600
    p.clipAudio = QColor("#7c3aed");        // violet-600
    p.clipText = QColor("#059669");         // emerald-600
    p.clipGraphic = QColor("#db2777");      // pink-600
    p.clipEffect = QColor("#0284c7");       // sky-600
    p.clipBookmark = QColor("#0284c7");     // sky-600
    p.waveformColor = QColor(255, 255, 255, 220);
    return p;
}
#endif

} // namespace catchim::ui
