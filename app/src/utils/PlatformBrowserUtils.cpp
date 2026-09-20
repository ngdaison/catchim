// Feature 223 -- mirrors web/src/utils/platform.ts + web/src/utils/browser.ts
#include "utils/PlatformBrowserUtils.h"
#include <regex>

namespace catchim::utils {

bool isAppleDevice() {
#if defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

std::string getPlatformSpecialKey() {
    return isAppleDevice() ? "\xE2\x8C\x98" : "Ctrl"; // ⌘
}

std::string getPlatformAlternateKey() {
    return isAppleDevice() ? "\xE2\x8C\xA5" : "Alt"; // ⌥
}

bool isTypableDOMElement(DOMElementTag tag, bool isDisabled) {
    switch (tag) {
        case DOMElementTag::ContentEditable:
            return true;
        case DOMElementTag::Input:
        case DOMElementTag::Textarea:
            return !isDisabled;
        default:
            return false;
    }
}

bool isScrollableOverflow(const std::string& overflow, const std::string& overflowX) {
    std::regex scrollPattern("auto|scroll");
    return std::regex_search(overflow + overflowX, scrollPattern);
}

} // namespace catchim::utils
