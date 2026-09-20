#include "CommandHistory.h"

namespace catchim::editor {

CommandHistory::CommandHistory(size_t maxHistory)
    : maxHistory_(maxHistory)
{
}

bool CommandHistory::execute(std::unique_ptr<EditorCommand> command) {
    if (!command) return false;

    if (!command->execute()) {
        return false;
    }

    // Discard any redos after current undo index
    if (undoIndex_ < commands_.size()) {
        commands_.erase(commands_.begin() + undoIndex_, commands_.end());
    }

    commands_.push_back(std::move(command));

    // Cap history
    if (commands_.size() > maxHistory_) {
        commands_.erase(commands_.begin());
    } else {
        undoIndex_++;
    }

    notifyChanged();
    return true;
}

bool CommandHistory::undo() {
    if (!canUndo()) return false;

    undoIndex_--;
    bool ok = commands_[undoIndex_]->undo();
    notifyChanged();
    return ok;
}

bool CommandHistory::redo() {
    if (!canRedo()) return false;

    bool ok = commands_[undoIndex_]->execute();
    undoIndex_++;
    notifyChanged();
    return ok;
}

void CommandHistory::clear() {
    commands_.clear();
    undoIndex_ = 0;
    notifyChanged();
}

void CommandHistory::notifyChanged() {
    if (onChanged_) {
        onChanged_();
    }
}

} // namespace catchim::editor
