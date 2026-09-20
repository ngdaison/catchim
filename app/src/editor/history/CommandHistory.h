#pragma once

#include "Command.h"
#include <vector>
#include <memory>
#include <functional>

namespace catchim::editor {

class CommandHistory {
public:
    explicit CommandHistory(size_t maxHistory = 100);

    bool execute(std::unique_ptr<EditorCommand> command);
    bool undo();
    bool redo();

    bool canUndo() const noexcept { return undoIndex_ > 0; }
    bool canRedo() const noexcept { return undoIndex_ < commands_.size(); }

    void clear();

    size_t size() const noexcept { return commands_.size(); }
    size_t undoIndex() const noexcept { return undoIndex_; }

    void setOnChangedCallback(std::function<void()> callback) {
        onChanged_ = std::move(callback);
    }

private:
    void notifyChanged();

    size_t maxHistory_{100};
    size_t undoIndex_{0};
    std::vector<std::unique_ptr<EditorCommand>> commands_;
    std::function<void()> onChanged_;
};

} // namespace catchim::editor
