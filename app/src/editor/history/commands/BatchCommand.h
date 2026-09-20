#pragma once

#include "editor/history/Command.h"
#include "editor/timeline/Timeline.h"
#include <vector>
#include <memory>
#include <string>

namespace catchim::editor {

/**
 * @brief Groups multiple EditorCommand instances into a single atomic transaction.
 *
 * Forward execution runs all child commands in order.
 * Rollback undoes all successfully executed child commands in reverse order.
 * Corresponds to web/src/commands/batch-command.ts.
 */
class BatchCommand : public EditorCommand {
public:
    BatchCommand();
    explicit BatchCommand(std::string name);
    explicit BatchCommand(std::vector<std::unique_ptr<EditorCommand>> commands, std::string name = "Batch Command");

    void addCommand(std::unique_ptr<EditorCommand> command);

    bool execute() override;
    bool undo() override;
    std::string name() const override { return name_; }

    size_t count() const noexcept { return commands_.size(); }
    bool empty() const noexcept { return commands_.empty(); }

    const std::vector<std::unique_ptr<EditorCommand>>& commands() const noexcept { return commands_; }

private:
    std::vector<std::unique_ptr<EditorCommand>> commands_;
    std::string name_;
    size_t executedCount_{0};
};

/**
 * @brief Restores the entire track hierarchy to a snapshot state on execute/undo.
 * Corresponds to web/src/commands/timeline/tracks-snapshot.ts.
 */
class TracksSnapshotCommand : public EditorCommand {
public:
    TracksSnapshotCommand(
        Timeline& timeline,
        TimelineTracksSnapshot before,
        TimelineTracksSnapshot after,
        std::string name = "Tracks Snapshot"
    );

    bool execute() override;
    bool undo() override;
    std::string name() const override { return name_; }

    const TimelineTracksSnapshot& beforeSnapshot() const noexcept { return before_; }
    const TimelineTracksSnapshot& afterSnapshot() const noexcept { return after_; }

private:
    Timeline& timeline_;
    TimelineTracksSnapshot before_;
    TimelineTracksSnapshot after_;
    std::string name_;
};

} // namespace catchim::editor
