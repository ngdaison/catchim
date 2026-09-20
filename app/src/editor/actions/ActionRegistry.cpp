#include "editor/actions/ActionRegistry.h"
#include "core/logging/Logger.h"
#include <algorithm>

namespace catchim::editor {

ActionRegistry& ActionRegistry::instance() {
    static ActionRegistry s_instance;
    return s_instance;
}

ActionRegistry::ActionRegistry() {
    registerDefaultActions();
}

void ActionRegistry::registerAction(const ActionDefinition& def) {
    m_actions[def.id] = def;
    for (const auto& shortcut : def.defaultShortcuts) {
        m_shortcutToAction[shortcut] = def.id;
    }
}

void ActionRegistry::bindAction(const std::string& actionId, ActionHandler handler) {
    m_handlers[actionId].push_back(std::move(handler));
}

void ActionRegistry::unbindAction(const std::string& actionId) {
    m_handlers.erase(actionId);
}

bool ActionRegistry::invokeAction(const std::string& actionId) {
    auto it = m_handlers.find(actionId);
    if (it == m_handlers.end() || it->second.empty()) {
        LOG_WARN("Action invoked but has no handlers bound: {}", actionId);
        return false;
    }

    for (auto& handler : it->second) {
        if (handler) {
            handler();
        }
    }
    return true;
}

bool ActionRegistry::invokeShortcut(const std::string& shortcut) {
    auto it = m_shortcutToAction.find(shortcut);
    if (it == m_shortcutToAction.end()) {
        return false;
    }
    return invokeAction(it->second);
}

void ActionRegistry::setShortcut(const std::string& shortcut, const std::string& actionId) {
    m_shortcutToAction[shortcut] = actionId;
}

std::optional<std::string> ActionRegistry::getActionForShortcut(const std::string& shortcut) const {
    auto it = m_shortcutToAction.find(shortcut);
    if (it != m_shortcutToAction.end()) {
        return it->second;
    }
    return std::nullopt;
}

const ActionDefinition* ActionRegistry::findAction(const std::string& actionId) const {
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        return &it->second;
    }
    return nullptr;
}

void ActionRegistry::registerDefaultActions() {
    registerAction({"toggle-play", "Play/Pause", ActionCategory::Playback, {"space", "k"}});
    registerAction({"stop-playback", "Stop playback", ActionCategory::Playback, {}});
    registerAction({"seek-forward", "Seek forward 1 second", ActionCategory::Playback, {"l"}});
    registerAction({"seek-backward", "Seek backward 1 second", ActionCategory::Playback, {"j"}});
    
    registerAction({"frame-step-forward", "Frame step forward", ActionCategory::Navigation, {"right"}});
    registerAction({"frame-step-backward", "Frame step backward", ActionCategory::Navigation, {"left"}});
    registerAction({"jump-forward", "Jump forward 5 seconds", ActionCategory::Navigation, {"shift+right"}});
    registerAction({"jump-backward", "Jump backward 5 seconds", ActionCategory::Navigation, {"shift+left"}});
    registerAction({"goto-start", "Go to timeline start", ActionCategory::Navigation, {"home", "enter"}});
    registerAction({"goto-end", "Go to timeline end", ActionCategory::Navigation, {"end"}});

    registerAction({"split", "Split elements at playhead", ActionCategory::Editing, {"s"}});
    registerAction({"split-left", "Split and remove left", ActionCategory::Editing, {"q"}});
    registerAction({"split-right", "Split and remove right", ActionCategory::Editing, {"w"}});
    registerAction({"delete-selected", "Delete current selection", ActionCategory::Editing, {"backspace", "delete"}});
    registerAction({"copy-selected", "Copy selected elements", ActionCategory::Editing, {"ctrl+c"}});
    registerAction({"paste-copied", "Paste elements at playhead", ActionCategory::Editing, {"ctrl+v"}});
    registerAction({"toggle-snapping", "Toggle snapping", ActionCategory::Editing, {"n"}});
    registerAction({"toggle-ripple-editing", "Toggle ripple editing", ActionCategory::Editing, {}});
    registerAction({"toggle-source-audio", "Extract or recover source audio", ActionCategory::Editing, {}});

    registerAction({"select-all", "Select all elements", ActionCategory::Selection, {"ctrl+a"}});
    registerAction({"deselect-all", "Deselect all elements", ActionCategory::Selection, {}});
    registerAction({"duplicate-selected", "Duplicate selected element", ActionCategory::Selection, {"ctrl+d"}});
    registerAction({"cancel-interaction", "Cancel current interaction", ActionCategory::Controls, {"escape"}});

    registerAction({"toggle-bookmark", "Toggle bookmark at playhead", ActionCategory::Timeline, {}});

    registerAction({"undo", "Undo", ActionCategory::History, {"ctrl+z"}});
    registerAction({"redo", "Redo", ActionCategory::History, {"ctrl+shift+z", "ctrl+y"}});
}

} // namespace catchim::editor
