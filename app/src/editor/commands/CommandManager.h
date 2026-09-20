#pragma once

#include "editor/history/Command.h"
#include "editor/selection/EditorSelection.h"
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace catchim::editor {

class EditorCore;

struct CommandHistoryEntry {
    std::shared_ptr<EditorCommand> command;
    EditorSelectionSnapshot previousSelection;
    std::optional<EditorSelectionSnapshot> selectionOverride{std::nullopt};
};

class CommandManager {
public:
    explicit CommandManager(EditorCore& editor);
    ~CommandManager() = default;

    CommandManager(const CommandManager&) = delete;
    CommandManager& operator=(const CommandManager&) = delete;
    CommandManager(CommandManager&&) = default;
    CommandManager& operator=(CommandManager&&) = default;

    bool isRippleEnabled{false};

    bool execute(
        std::shared_ptr<EditorCommand> command,
        std::optional<EditorSelectionSnapshot> selectionOverride = std::nullopt);

    void push(std::shared_ptr<EditorCommand> command);

    bool undo();
    bool redo();

    bool canUndo() const noexcept { return !history_.empty(); }
    bool canRedo() const noexcept { return !redoStack_.empty(); }
    void clear() noexcept;

    size_t historyCount() const noexcept { return history_.size(); }
    size_t redoCount() const noexcept { return redoStack_.size(); }

    void registerReactor(std::function<void()> reactor);
    void runReactors();

private:
    EditorSelectionSnapshot getSelectionSnapshot() const;

    EditorCore& editor_;
    std::vector<CommandHistoryEntry> history_;
    std::vector<CommandHistoryEntry> redoStack_;
    std::vector<std::function<void()>> reactors_;
};

} // namespace catchim::editor
