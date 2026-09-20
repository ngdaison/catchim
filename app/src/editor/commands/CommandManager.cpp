#include "CommandManager.h"
#include "editor/core/EditorCore.h"

namespace catchim::editor {

CommandManager::CommandManager(EditorCore& editor)
    : editor_(editor) {
}

EditorSelectionSnapshot CommandManager::getSelectionSnapshot() const {
    return editor_.selection().getSnapshot();
}

bool CommandManager::execute(
    std::shared_ptr<EditorCommand> command,
    std::optional<EditorSelectionSnapshot> selectionOverride) {

    if (!command) {
        return false;
    }

    const auto previousSelection = getSelectionSnapshot();
    const bool success = command->execute();
    if (!success) {
        return false;
    }

    runReactors();

    history_.push_back(CommandHistoryEntry{
        command,
        previousSelection,
        selectionOverride
    });
    redoStack_.clear();
    return true;
}

void CommandManager::push(std::shared_ptr<EditorCommand> command) {
    if (!command) return;
    history_.push_back(CommandHistoryEntry{
        command,
        getSelectionSnapshot(),
        std::nullopt
    });
    redoStack_.clear();
}

bool CommandManager::undo() {
    if (history_.empty()) {
        return false;
    }

    auto entry = std::move(history_.back());
    history_.pop_back();

    bool success = false;
    if (entry.command) {
        success = entry.command->undo();
    }

    if (entry.selectionOverride.has_value()) {
        editor_.selection().restoreSnapshot(entry.previousSelection);
    }

    redoStack_.push_back(std::move(entry));
    runReactors();
    return success;
}

bool CommandManager::redo() {
    if (redoStack_.empty()) {
        return false;
    }

    auto entry = std::move(redoStack_.back());
    redoStack_.pop_back();

    bool success = false;
    if (entry.command) {
        success = entry.command->execute();
    }

    if (entry.selectionOverride.has_value()) {
        editor_.selection().restoreSnapshot(*entry.selectionOverride);
    }

    history_.push_back(std::move(entry));
    runReactors();
    return success;
}

void CommandManager::clear() noexcept {
    history_.clear();
    redoStack_.clear();
}

void CommandManager::registerReactor(std::function<void()> reactor) {
    if (reactor) {
        reactors_.push_back(std::move(reactor));
    }
}

void CommandManager::runReactors() {
    for (const auto& reactor : reactors_) {
        if (reactor) {
            reactor();
        }
    }
}

} // namespace catchim::editor
