#include "EditableTimecodeController.h"

namespace catchim::editor {

void EditableTimecodeController::startEditing(const std::string& formattedTime) noexcept {
    isEditing_ = true;
    hasError_ = false;
    inputValue_ = formattedTime;
}

void EditableTimecodeController::setInputValue(const std::string& text) noexcept {
    inputValue_ = text;
    hasError_ = false;
}

void EditableTimecodeController::cancelEditing() noexcept {
    isEditing_ = false;
    hasError_ = false;
    inputValue_.clear();
}

std::optional<core::TimelineTime> EditableTimecodeController::applyEdit(
    core::TimecodeFormat format,
    const core::FrameRate& fps,
    core::TimelineTime duration
) {
    auto parsedOpt = core::Timecode::parse(inputValue_, format, fps);
    if (!parsedOpt.has_value()) {
        hasError_ = true;
        return std::nullopt;
    }

    hasError_ = false;
    isEditing_ = false;

    core::TimelineTime result = *parsedOpt;
    if (duration > core::TimelineTime::zero()) {
        result = result.snappedSeek(duration, fps);
    } else {
        result = result.roundToFrame(fps);
    }

    return result;
}

std::string EditableTimecodeController::formatTime(
    core::TimelineTime time,
    core::TimecodeFormat format,
    const core::FrameRate& fps
) {
    return core::Timecode::format(time, format, fps);
}

} // namespace catchim::editor
