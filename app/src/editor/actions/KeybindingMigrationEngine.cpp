#include "editor/actions/KeybindingMigrationEngine.h"

namespace catchim::editor {

std::optional<PersistedKeybindingsState> KeybindingMigrationEngine::parsePersistedKeybindingsState(
    const nlohmann::json& state
) {
    if (!state.is_object()) {
        return std::nullopt;
    }

    if (!state.contains("keybindings") || !state["keybindings"].is_object()) {
        return std::nullopt;
    }

    if (!state.contains("isCustomized") || !state["isCustomized"].is_boolean()) {
        return std::nullopt;
    }

    PersistedKeybindingsState result;
    result.isCustomized = state["isCustomized"].get<bool>();

    for (auto it = state["keybindings"].begin(); it != state["keybindings"].end(); ++it) {
        if (!it.value().is_string()) {
            return std::nullopt;
        }
        result.keybindings[it.key()] = it.value().get<std::string>();
    }

    return result;
}

nlohmann::json KeybindingMigrationEngine::serializePersistedKeybindingsState(
    const PersistedKeybindingsState& state
) {
    nlohmann::json j;
    j["keybindings"] = state.keybindings;
    j["isCustomized"] = state.isCustomized;
    return j;
}

nlohmann::json KeybindingMigrationEngine::v2ToV3(const nlohmann::json& state) {
    auto parsed = parsePersistedKeybindingsState(state);
    if (!parsed) return state;

    static const std::map<std::string, std::string> renames = {
        {"split-selected", "split"},
        {"split-selected-left", "split-left"},
        {"split-selected-right", "split-right"}
    };

    for (auto& [key, action] : parsed->keybindings) {
        const auto it = renames.find(action);
        if (it != renames.end()) {
            action = it->second;
        }
    }

    return serializePersistedKeybindingsState(*parsed);
}

nlohmann::json KeybindingMigrationEngine::v3ToV4(const nlohmann::json& state) {
    auto parsed = parsePersistedKeybindingsState(state);
    if (!parsed) return state;

    static const std::map<std::string, std::string> renames = {
        {"paste-selected", "paste-copied"}
    };

    for (auto& [key, action] : parsed->keybindings) {
        const auto it = renames.find(action);
        if (it != renames.end()) {
            action = it->second;
        }
    }

    return serializePersistedKeybindingsState(*parsed);
}

nlohmann::json KeybindingMigrationEngine::v4ToV5(const nlohmann::json& state) {
    auto parsed = parsePersistedKeybindingsState(state);
    if (!parsed) return state;

    if (parsed->keybindings.find("escape") == parsed->keybindings.end()) {
        parsed->keybindings["escape"] = "deselect-all";
    }

    return serializePersistedKeybindingsState(*parsed);
}

nlohmann::json KeybindingMigrationEngine::v5ToV6(const nlohmann::json& state) {
    auto parsed = parsePersistedKeybindingsState(state);
    if (!parsed) return state;

    auto it = parsed->keybindings.find("escape");
    if (it != parsed->keybindings.end() && it->second == "deselect-all") {
        it->second = "cancel-interaction";
    }

    return serializePersistedKeybindingsState(*parsed);
}

nlohmann::json KeybindingMigrationEngine::v6ToV7(const nlohmann::json& state) {
    auto parsed = parsePersistedKeybindingsState(state);
    if (!parsed) return state;

    for (auto& [key, action] : parsed->keybindings) {
        if (action == "split-element") {
            action = "split";
        }
    }

    return serializePersistedKeybindingsState(*parsed);
}

nlohmann::json KeybindingMigrationEngine::runMigrations(
    const nlohmann::json& state,
    int fromVersion
) {
    nlohmann::json current = state;
    for (int version = fromVersion; version < CURRENT_KEYBINDINGS_VERSION; ++version) {
        switch (version) {
            case 2: current = v2ToV3(current); break;
            case 3: current = v3ToV4(current); break;
            case 4: current = v4ToV5(current); break;
            case 5: current = v5ToV6(current); break;
            case 6: current = v6ToV7(current); break;
            default: break;
        }
    }
    return current;
}

} // namespace catchim::editor
