#pragma once

#include "Colors.h"
#include "Metrics.h"

#if defined(HAVE_QT6)
#include <QFont>
#include <QString>
#include <functional>
#include <unordered_map>
#include <string>

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

    using ThemeListener = std::function<void(ThemeMode)>;
    void addListener(const std::string& key, ThemeListener listener);
    void removeListener(const std::string& key);

private:
    Theme();
    ThemeMode mode_{ThemeMode::Dark};
    Palette palette_;
    Metrics metrics_;
    std::unordered_map<std::string, ThemeListener> listeners_;
};

} // namespace catchim::ui
#endif
