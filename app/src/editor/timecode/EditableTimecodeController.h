#pragma once

#include "core/time/TimelineTime.h"
#include "core/time/Timecode.h"
#include <string>
#include <optional>

namespace catchim::editor {

class EditableTimecodeController {
public:
    EditableTimecodeController() = default;

    [[nodiscard]] bool isEditing() const noexcept { return isEditing_; }
    [[nodiscard]] bool hasError() const noexcept { return hasError_; }
    [[nodiscard]] const std::string& inputValue() const noexcept { return inputValue_; }

    void startEditing(const std::string& formattedTime) noexcept;
    void setInputValue(const std::string& text) noexcept;
    void cancelEditing() noexcept;

    std::optional<core::TimelineTime> applyEdit(
        core::TimecodeFormat format = core::TimecodeFormat::HH_MM_SS_FF,
        const core::FrameRate& fps = {30, 1},
        core::TimelineTime duration = core::TimelineTime::zero()
    );

    static std::string formatTime(
        core::TimelineTime time,
        core::TimecodeFormat format = core::TimecodeFormat::HH_MM_SS_FF,
        const core::FrameRate& fps = {30, 1}
    );

private:
    bool isEditing_{false};
    bool hasError_{false};
    std::string inputValue_;
};

} // namespace catchim::editor
