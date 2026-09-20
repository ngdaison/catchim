#include "editor/actions/KeybindingEngine.h"
#include <unordered_set>
#include <algorithm>
#include <sstream>

namespace catchim::editor {

static const std::unordered_set<std::string> kValidKeys = {
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j",
    "k", "l", "m", "n", "o", "p", "q", "r", "s", "t",
    "u", "v", "w", "x", "y", "z",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "up", "down", "left", "right",
    "/", "?", ".", "+", "-", "=",
    "enter", "tab", "space", "escape",
    "backspace", "delete", "home", "end"
};

bool KeybindingEngine::isKey(std::string_view key) noexcept {
    std::string lower(key);
    for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return kValidKeys.contains(lower);
}

bool KeybindingEngine::isModifier(std::string_view mod) noexcept {
    std::string lower(mod);
    for (char& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return lower == "ctrl" || lower == "cmd" || lower == "meta" || lower == "control" ||
           lower == "alt" || lower == "opt" || lower == "option" ||
           lower == "shift";
}

std::string KeybindingEngine::normalizeShortcut(std::string_view raw) {
    if (raw.empty()) return "";

    std::string lower;
    lower.reserve(raw.size());
    for (char c : raw) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    // Split by '+'
    std::vector<std::string> tokens;
    std::string current;
    for (size_t i = 0; i < lower.size(); ++i) {
        char c = lower[i];
        if (c == '+') {
            // Handle edge case where '+' is itself the key (e.g. "ctrl++")
            if (current.empty() && i > 0 && lower[i - 1] == '+') {
                current.push_back('+');
            } else if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else if (!std::isspace(static_cast<unsigned char>(c))) {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }

    if (tokens.empty()) return "";

    bool hasCtrl = false;
    bool hasAlt = false;
    bool hasShift = false;
    std::string keyToken;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& tok = tokens[i];
        if (tok == "ctrl" || tok == "cmd" || tok == "meta" || tok == "control") {
            hasCtrl = true;
        } else if (tok == "alt" || tok == "opt" || tok == "option") {
            hasAlt = true;
        } else if (tok == "shift") {
            hasShift = true;
        } else {
            // Normalize aliases
            if (tok == "esc") {
                keyToken = "escape";
            } else {
                keyToken = tok;
            }
        }
    }

    if (keyToken.empty()) {
        return "";
    }

    std::string result;
    if (hasCtrl) result += "ctrl+";
    if (hasAlt) result += "alt+";
    if (hasShift) result += "shift+";
    result += keyToken;

    return result;
}

bool KeybindingEngine::isShortcutKey(std::string_view str) noexcept {
    std::string norm = normalizeShortcut(str);
    if (norm.empty()) return false;

    auto plusPos = norm.rfind('+');
    if (plusPos == std::string::npos) {
        return isKey(norm);
    }

    std::string keyPart = norm.substr(plusPos + 1);
    return isKey(keyPart);
}

bool KeybindingEngine::isSingleCharacterShortcut(std::string_view shortcut) noexcept {
    std::string norm = normalizeShortcut(shortcut);
    return !norm.empty() && norm.find('+') == std::string::npos;
}

bool KeybindingEngine::isModifierBasedShortcut(std::string_view shortcut) noexcept {
    std::string norm = normalizeShortcut(shortcut);
    return !norm.empty() && norm.find('+') != std::string::npos;
}

std::optional<KeybindingConflict> KeybindingEngine::validateKeybinding(
    const std::unordered_map<std::string, std::string>& currentMap,
    std::string_view shortcut,
    std::string_view newAction
) {
    std::string norm = normalizeShortcut(shortcut);
    if (norm.empty()) return std::nullopt;

    auto it = currentMap.find(norm);
    if (it != currentMap.end()) {
        if (it->second != newAction) {
            return KeybindingConflict{norm, it->second, std::string(newAction)};
        }
    }
    return std::nullopt;
}

std::unordered_map<std::string, std::string> KeybindingEngine::getDefaultShortcuts() {
    return {
        {"space", "toggle-play"},
        {"k", "toggle-play"},
        {"j", "seek-backward"},
        {"l", "seek-forward"},
        {"left", "frame-step-backward"},
        {"right", "frame-step-forward"},
        {"shift+left", "jump-backward"},
        {"shift+right", "jump-forward"},
        {"home", "goto-start"},
        {"end", "goto-end"},
        {"s", "split"},
        {"q", "split-left"},
        {"w", "split-right"},
        {"delete", "delete-selected"},
        {"backspace", "delete-selected"},
        {"ctrl+c", "copy-selected"},
        {"ctrl+v", "paste-copied"},
        {"n", "toggle-snapping"},
        {"r", "toggle-ripple-editing"},
        {"m", "toggle-bookmark"},
        {"ctrl+z", "undo"},
        {"ctrl+shift+z", "redo"},
        {"ctrl+y", "redo"},
        {"ctrl++", "zoom-in"},
        {"ctrl+-", "zoom-out"},
        {"ctrl+0", "fit-to-screen"}
    };
}

nlohmann::json KeybindingEngine::exportConfig(
    const std::unordered_map<std::string, std::string>& shortcuts
) {
    nlohmann::json root = nlohmann::json::object();
    for (const auto& [sc, action] : shortcuts) {
        root[sc] = action;
    }
    return root;
}

std::unordered_map<std::string, std::string> KeybindingEngine::importConfig(
    const nlohmann::json& jsonConfig
) {
    std::unordered_map<std::string, std::string> result;
    if (!jsonConfig.is_object()) {
        return result;
    }

    for (auto it = jsonConfig.begin(); it != jsonConfig.end(); ++it) {
        if (it.value().is_string()) {
            std::string norm = normalizeShortcut(it.key());
            if (!norm.empty()) {
                result[norm] = it.value().get<std::string>();
            }
        }
    }
    return result;
}

} // namespace catchim::editor
