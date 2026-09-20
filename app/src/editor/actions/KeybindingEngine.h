#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <string_view>
#include <nlohmann/json.hpp>

namespace catchim::editor {

struct KeybindingConflict {
    std::string key;
    std::string existingAction;
    std::string newAction;

    bool operator==(const KeybindingConflict& other) const = default;
};

class KeybindingEngine {
public:
    static bool isKey(std::string_view key) noexcept;
    static bool isModifier(std::string_view mod) noexcept;

    // Normalizes shortcut string: lowercases, converts cmd->ctrl, option->alt, sorts modifiers: ctrl+alt+shift+key
    static std::string normalizeShortcut(std::string_view raw);

    static bool isShortcutKey(std::string_view str) noexcept;
    static bool isSingleCharacterShortcut(std::string_view shortcut) noexcept;
    static bool isModifierBasedShortcut(std::string_view shortcut) noexcept;

    // Checks whether assigning this shortcut to newAction conflicts with an existing assignment
    static std::optional<KeybindingConflict> validateKeybinding(
        const std::unordered_map<std::string, std::string>& currentMap,
        std::string_view shortcut,
        std::string_view newAction
    );

    // Default application shortcuts mapping: shortcut -> actionId
    static std::unordered_map<std::string, std::string> getDefaultShortcuts();

    // Export/import mappings from JSON
    static nlohmann::json exportConfig(const std::unordered_map<std::string, std::string>& shortcuts);
    static std::unordered_map<std::string, std::string> importConfig(const nlohmann::json& jsonConfig);
};

} // namespace catchim::editor
