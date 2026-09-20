#pragma once
// Feature 223 -- mirrors web/src/utils/platform.ts + web/src/utils/browser.ts
#include <string>

namespace catchim::utils {

// ---- Platform (platform.ts) ----

// Returns true if running on an Apple device (Mac/iPhone/iPad/iPod)
// In C++ context: checks compile-time platform via preprocessor
bool isAppleDevice();

// Returns the platform-specific special key label (Cmd or Ctrl)
std::string getPlatformSpecialKey();

// Returns the platform-specific alternate key label (Option/Alt)
std::string getPlatformAlternateKey();

// ---- Browser DOM utilities (browser.ts) ----

// Checks if MIME string corresponds to a typeable/input element tag
// (C++ equivalent: checks whether an element tag is input/textarea/contenteditable)
enum class DOMElementTag { Input, Textarea, ContentEditable, Other };

bool isTypableDOMElement(DOMElementTag tag, bool isDisabled = false);

// Checks if an element's CSS overflow contains scroll/auto
// (simplified: given overflow strings)
bool isScrollableOverflow(const std::string& overflow, const std::string& overflowX);

} // namespace catchim::utils
