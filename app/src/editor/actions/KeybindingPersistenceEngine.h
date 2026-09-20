#pragma once
// Feature 223 -- mirrors web/src/actions/keybindings/persistence.ts
#include "editor/actions/KeybindingMigrationEngine.h"
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace catchim::editor {

struct DecodedKeybindingsState {
    std::map<std::string, std::string> keybindings;
    bool isCustomized{false};

    bool operator==(const DecodedKeybindingsState& other) const noexcept {
        return isCustomized == other.isCustomized && keybindings == other.keybindings;
    }
};

class KeybindingPersistenceEngine {
public:
    static bool isValidShortcutKey(const std::string& key);
    static bool isValidAction(const std::string& action);

    static nlohmann::json serializeKeybindingsState(const DecodedKeybindingsState& state);

    static nlohmann::json migratePersistedKeybindingsState(const nlohmann::json& state,
                                                          int fromVersion);

    // Lossy decode: ignores unrecognizable structure (returns nullopt), drops invalid entries
    static std::optional<DecodedKeybindingsState> decodePersistedKeybindingsState(
        const nlohmann::json& state,
        std::vector<std::string>* outDroppedKeys = nullptr
    );

    // Strict parse: throws std::runtime_error on invalid structure or values
    static std::map<std::string, std::string> parseImportedKeybindings(
        const nlohmann::json& config
    );
};

} // namespace catchim::editor
