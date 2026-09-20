#pragma once

#include <string>

namespace catchim::editor {

class EditorCommand {
public:
    virtual ~EditorCommand() = default;

    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual std::string name() const = 0;
};

} // namespace catchim::editor
