#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <optional>

namespace catchim::editor {

constexpr int CURRENT_KEYBINDINGS_VERSION = 7;

struct PersistedKeybindingsState {
    std::map<std::string, std::string> keybindings;
    bool isCustomized{false};

    bool operator==(const PersistedKeybindingsState& other) const noexcept {
        return isCustomized == other.isCustomized && keybindings == other.keybindings;
    }
};

class KeybindingMigrationEngine {
public:
    static std::optional<PersistedKeybindingsState> parsePersistedKeybindingsState(
        const nlohmann::json& state
    );

    static nlohmann::json serializePersistedKeybindingsState(
        const PersistedKeybindingsState& state
    );

    static nlohmann::json v2ToV3(const nlohmann::json& state);
    static nlohmann::json v3ToV4(const nlohmann::json& state);
    static nlohmann::json v4ToV5(const nlohmann::json& state);
    static nlohmann::json v5ToV6(const nlohmann::json& state);
    static nlohmann::json v6ToV7(const nlohmann::json& state);

    static nlohmann::json runMigrations(
        const nlohmann::json& state,
        int fromVersion
    );
};

} // namespace catchim::editor
