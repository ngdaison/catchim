#pragma once

#include "Colors.h"
#include "Metrics.h"

#if defined(HAVE_QT6)
#include <QFont>
#include <QString>

namespace catchim::ui {

enum class ThemeMode {
    Dark,
    Light
};

class Theme {
public:
    static Theme& instance();

    void setTheme(ThemeMode mode);
    ThemeMode mode() const noexcept { return mode_; }

    const Palette& palette() const noexcept { return palette_; }
    const Metrics& metrics() const noexcept { return metrics_; }

    QFont fontSans(int pointSize = 10, int weight = QFont::Normal) const;
    QFont fontMono(int pointSize = 9) const;

    QString buildGlobalStyleSheet() const;

private:
    Theme();
    ThemeMode mode_{ThemeMode::Dark};
    Palette palette_;
    Metrics metrics_;
};

} // namespace catchim::ui
#endif
