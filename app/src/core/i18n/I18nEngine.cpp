#include "core/i18n/I18nEngine.h"
#include <algorithm>

namespace catchim::core {

I18nEngine::I18nEngine() {
    loadDefaultTranslations();
}

I18nEngine& I18nEngine::instance() {
    static I18nEngine s_instance;
    return s_instance;
}

bool I18nEngine::setLocale(const std::string& locale) {
    auto it = std::find(supportedLocales_.begin(), supportedLocales_.end(), locale);
    if (it == supportedLocales_.end()) {
        return false;
    }
    if (currentLocale_ != locale) {
        currentLocale_ = locale;
        for (const auto& listener : listeners_) {
            if (listener) {
                listener(currentLocale_);
            }
        }
    }
    return true;
}

std::string I18nEngine::t(
    const std::string& key,
    const std::map<std::string, std::string>& params
) const {
    // 1. Try current locale
    auto locIt = dictionary_.find(currentLocale_);
    if (locIt != dictionary_.end()) {
        auto keyIt = locIt->second.find(key);
        if (keyIt != locIt->second.end()) {
            return interpolate(keyIt->second, params);
        }
    }

    // 2. Fallback to "en"
    if (currentLocale_ != "en") {
        auto enIt = dictionary_.find("en");
        if (enIt != dictionary_.end()) {
            auto keyIt = enIt->second.find(key);
            if (keyIt != enIt->second.end()) {
                return interpolate(keyIt->second, params);
            }
        }
    }

    // 3. Fallback to key itself
    return interpolate(key, params);
}

bool I18nEngine::hasKey(const std::string& key, const std::string& locale) const noexcept {
    const std::string& targetLoc = locale.empty() ? currentLocale_ : locale;
    auto locIt = dictionary_.find(targetLoc);
    if (locIt == dictionary_.end()) {
        return false;
    }
    return locIt->second.contains(key);
}

void I18nEngine::addTranslation(
    const std::string& locale,
    const std::string& key,
    std::string value
) {
    dictionary_[locale][key] = std::move(value);
}

void I18nEngine::addListener(LocaleChangeListener listener) {
    if (listener) {
        listeners_.push_back(std::move(listener));
    }
}

std::string I18nEngine::interpolate(
    const std::string& templateStr,
    const std::map<std::string, std::string>& params
) {
    if (params.empty()) {
        return templateStr;
    }

    std::string result = templateStr;
    for (const auto& [paramName, paramVal] : params) {
        const std::string placeholder = "{" + paramName + "}";
        size_t pos = 0;
        while ((pos = result.find(placeholder, pos)) != std::string::npos) {
            result.replace(pos, placeholder.length(), paramVal);
            pos += paramVal.length();
        }
    }
    return result;
}

void I18nEngine::loadDefaultTranslations() {
    // English defaults (from web/src/i18n/locales/en.ts)
    auto& en = dictionary_["en"];
    en["common.apply"] = "Apply";
    en["common.cancel"] = "Cancel";
    en["common.close"] = "Close";
    en["common.copy"] = "Copy";
    en["common.save"] = "Save";
    en["common.loading"] = "Loading...";
    en["common.retry"] = "Retry";
    en["common.presets"] = "Presets";
    en["common.english"] = "English";
    en["common.vietnamese"] = "Vietnamese";

    en["settings.aspectRatio"] = "Aspect ratio";
    en["settings.frameRate"] = "Frame rate";
    en["settings.language"] = "Language";
    en["settings.background"] = "Background";
    en["settings.name"] = "Project name";

    en["assets.media"] = "Media";
    en["assets.effects"] = "Effects";
    en["assets.captions"] = "Captions";
    en["assets.soundEffects"] = "Sound Effects";
    en["assets.import"] = "Import";
    en["assets.duration"] = "Duration";

    en["timeline.split"] = "Split";
    en["timeline.delete"] = "Delete";
    en["timeline.ripple"] = "Ripple";
    en["timeline.zoomIn"] = "Zoom In";
    en["timeline.zoomOut"] = "Zoom Out";

    en["export.title"] = "Export Video";
    en["export.format"] = "Format";
    en["export.quality"] = "Quality";
    en["export.resolution"] = "Resolution";
    en["export.exporting"] = "Exporting {name}...";

    // Vietnamese defaults (from web/src/i18n/locales/vi.ts)
    auto& vi = dictionary_["vi"];
    vi["common.apply"] = "Áp dụng";
    vi["common.cancel"] = "Hủy";
    vi["common.close"] = "Đóng";
    vi["common.copy"] = "Sao chép";
    vi["common.save"] = "Lưu";
    vi["common.loading"] = "Đang tải...";
    vi["common.retry"] = "Thử lại";
    vi["common.presets"] = "Cài đặt sẵn";
    vi["common.english"] = "Tiếng Anh";
    vi["common.vietnamese"] = "Tiếng Việt";

    vi["settings.aspectRatio"] = "Tỉ lệ khung hình";
    vi["settings.frameRate"] = "Tốc độ khung hình";
    vi["settings.language"] = "Ngôn ngữ";
    vi["settings.background"] = "Hình nền";
    vi["settings.name"] = "Tên dự án";

    vi["assets.media"] = "Tệp phương tiện";
    vi["assets.effects"] = "Hiệu ứng";
    vi["assets.captions"] = "Phụ đề";
    vi["assets.soundEffects"] = "Hiệu ứng âm thanh";
    vi["assets.import"] = "Nhập tệp";
    vi["assets.duration"] = "Thời lượng";

    vi["timeline.split"] = "Cắt";
    vi["timeline.delete"] = "Xóa";
    vi["timeline.ripple"] = "Cuộn ripple";
    vi["timeline.zoomIn"] = "Phóng to";
    vi["timeline.zoomOut"] = "Thu nhỏ";

    vi["export.title"] = "Xuất Video";
    vi["export.format"] = "Định dạng";
    vi["export.quality"] = "Chất lượng";
    vi["export.resolution"] = "Độ phân giải";
    vi["export.exporting"] = "Đang xuất {name}...";
}

} // namespace catchim::core
