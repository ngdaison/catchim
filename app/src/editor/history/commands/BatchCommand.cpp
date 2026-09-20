#include "BatchCommand.h"

namespace catchim::editor {

BatchCommand::BatchCommand()
    : name_("Batch Command")
{
}

BatchCommand::BatchCommand(std::string name)
    : name_(std::move(name))
{
}

BatchCommand::BatchCommand(std::vector<std::unique_ptr<EditorCommand>> commands, std::string name)
    : commands_(std::move(commands))
    , name_(std::move(name))
{
}

void BatchCommand::addCommand(std::unique_ptr<EditorCommand> command) {
    if (command) {
        commands_.push_back(std::move(command));
    }
}

bool BatchCommand::execute() {
    executedCount_ = 0;
    for (size_t i = 0; i < commands_.size(); ++i) {
        if (!commands_[i]->execute()) {
            // Rollback already executed commands in reverse
            for (size_t j = executedCount_; j > 0; --j) {
                commands_[j - 1]->undo();
            }
            executedCount_ = 0;
            return false;
        }
        ++executedCount_;
    }
    return true;
}

bool BatchCommand::undo() {
    bool allSuccess = true;
    for (size_t i = executedCount_; i > 0; --i) {
        if (!commands_[i - 1]->undo()) {
            allSuccess = false;
        }
    }
    executedCount_ = 0;
    return allSuccess;
}

TracksSnapshotCommand::TracksSnapshotCommand(
    Timeline& timeline,
    TimelineTracksSnapshot before,
    TimelineTracksSnapshot after,
    std::string name
)
    : timeline_(timeline)
    , before_(std::move(before))
    , after_(std::move(after))
    , name_(std::move(name))
{
}

bool TracksSnapshotCommand::execute() {
    timeline_.restoreSnapshot(after_);
    return true;
}

bool TracksSnapshotCommand::undo() {
    timeline_.restoreSnapshot(before_);
    return true;
}

} // namespace catchim::editor
