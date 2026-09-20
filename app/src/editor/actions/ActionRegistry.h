#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>

namespace catchim::editor {

enum class ActionCategory {
    Playback,
    Navigation,
    Editing,
    Selection,
    History,
    Timeline,
    Controls,
    Assets
};

struct ActionDefinition {
    std::string id;
    std::string description;
    ActionCategory category;
    std::vector<std::string> defaultShortcuts;
};

using ActionHandler = std::function<void()>;

class ActionRegistry {
public:
    static ActionRegistry& instance();

    void registerAction(const ActionDefinition& def);
    void bindAction(const std::string& actionId, ActionHandler handler);
    void unbindAction(const std::string& actionId);
    bool invokeAction(const std::string& actionId);

    // Shortcut dispatch
    bool invokeShortcut(const std::string& shortcut);
    void setShortcut(const std::string& shortcut, const std::string& actionId);
    std::optional<std::string> getActionForShortcut(const std::string& shortcut) const;

    const std::unordered_map<std::string, ActionDefinition>& allActions() const { return m_actions; }
    const ActionDefinition* findAction(const std::string& actionId) const;

private:
    ActionRegistry();
    void registerDefaultActions();

    std::unordered_map<std::string, ActionDefinition> m_actions;
    std::unordered_map<std::string, std::vector<ActionHandler>> m_handlers;
    std::unordered_map<std::string, std::string> m_shortcutToAction;
};

} // namespace catchim::editor
