// Feature 223 -- mirrors web/src/actions/keybindings/persistence.ts
#include "editor/actions/KeybindingPersistenceEngine.h"
#include "editor/actions/KeybindingMigrationEngine.h"

namespace catchim::editor {

bool KeybindingPersistenceEngine::isValidShortcutKey(const std::string& key) {
    if (key.empty()) return false;
    // Shortcuts typically contain standard key tokens or modifiers like ctrl+, alt+, shift+, meta+, space, etc.
    return true;
}

bool KeybindingPersistenceEngine::isValidAction(const std::string& action) {
    if (action.empty()) return false;
    // Actions typically have format "action_name" or "action_name:arg"
    return true;
}

nlohmann::json KeybindingPersistenceEngine::serializeKeybindingsState(const DecodedKeybindingsState& state) {
    nlohmann::json j;
    j["keybindings"] = state.keybindings;
    j["isCustomized"] = state.isCustomized;
    return j;
}

nlohmann::json KeybindingPersistenceEngine::migratePersistedKeybindingsState(const nlohmann::json& state,
                                                                            int fromVersion) {
    return KeybindingMigrationEngine::runMigrations(state, fromVersion);
}

std::optional<DecodedKeybindingsState> KeybindingPersistenceEngine::decodePersistedKeybindingsState(
    const nlohmann::json& state,
    std::vector<std::string>* outDroppedKeys
) {
    auto parsed = KeybindingMigrationEngine::parsePersistedKeybindingsState(state);
    if (!parsed.has_value()) {
        return std::nullopt;
    }

    DecodedKeybindingsState result;
    result.isCustomized = parsed->isCustomized;

    for (const auto& [key, action] : parsed->keybindings) {
        if (!isValidShortcutKey(key) || !isValidAction(action)) {
            if (outDroppedKeys) {
                outDroppedKeys->push_back(key);
            }
            continue;
        }
        result.keybindings[key] = action;
    }

    return result;
}

std::map<std::string, std::string> KeybindingPersistenceEngine::parseImportedKeybindings(
    const nlohmann::json& config
) {
    if (!config.is_object()) {
        throw std::runtime_error("Imported keybindings must be a JSON object");
    }

    std::map<std::string, std::string> result;
    for (auto it = config.begin(); it != config.end(); ++it) {
        const std::string& key = it.key();
        if (!it.value().is_string()) {
            throw std::runtime_error("Invalid action for \"" + key + "\": expected string");
        }
        std::string action = it.value().get<std::string>();
        if (!isValidShortcutKey(key)) {
            throw std::runtime_error("Invalid shortcut key: " + key);
        }
        if (!isValidAction(action)) {
            throw std::runtime_error("Invalid action for \"" + key + "\": " + action);
        }
        result[key] = action;
    }
    return result;
}

} // namespace catchim::editor
