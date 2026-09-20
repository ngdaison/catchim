#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <optional>

namespace catchim::core {

/**
 * @brief Internationalization (I18n) Engine with multi-locale support (en, vi) and variable interpolation.
 * Corresponds to web/src/i18n/ (locales/en.ts, locales/vi.ts, translations.ts, types.ts).
 */
class I18nEngine {
public:
    using LocaleChangeListener = std::function<void(const std::string& newLocale)>;

    I18nEngine();

    static I18nEngine& instance();

    bool setLocale(const std::string& locale);
    const std::string& currentLocale() const noexcept { return currentLocale_; }
    const std::vector<std::string>& supportedLocales() const noexcept { return supportedLocales_; }

    std::string t(
        const std::string& key,
        const std::map<std::string, std::string>& params = {}
    ) const;

    bool hasKey(const std::string& key, const std::string& locale = "") const noexcept;

    void addTranslation(
        const std::string& locale,
        const std::string& key,
        std::string value
    );

    void addListener(LocaleChangeListener listener);

    static std::string interpolate(
        const std::string& templateStr,
        const std::map<std::string, std::string>& params
    );

private:
    void loadDefaultTranslations();

    std::string currentLocale_{"en"};
    std::vector<std::string> supportedLocales_{"en", "vi"};
    std::map<std::string, std::map<std::string, std::string>> dictionary_;
    std::vector<LocaleChangeListener> listeners_;
};

} // namespace catchim::core
