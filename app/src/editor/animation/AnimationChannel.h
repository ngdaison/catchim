#pragma once

#include "Keyframe.h"
#include <vector>
#include <optional>
#include <string>

namespace catchim::editor {

class AnimationChannel {
public:
    AnimationChannel() = default;
    explicit AnimationChannel(std::string propertyName, double defaultValue = 0.0)
        : m_propertyName(std::move(propertyName)), m_defaultValue(defaultValue) {}

    const std::string& propertyName() const noexcept { return m_propertyName; }
    double defaultValue() const noexcept { return m_defaultValue; }
    void setDefaultValue(double val) noexcept { m_defaultValue = val; }

    const std::vector<Keyframe>& keyframes() const noexcept { return m_keyframes; }
    std::vector<Keyframe>& keyframes() noexcept { return m_keyframes; }

    void addOrUpdateKeyframe(Keyframe kf);
    bool removeKeyframeAt(core::TimelineTime time, core::TimelineTime threshold = core::TimelineTime::fromTicks(100));
    std::optional<Keyframe> findKeyframeAt(core::TimelineTime time, core::TimelineTime threshold = core::TimelineTime::fromTicks(100)) const;

    double getValueAt(core::TimelineTime time) const;

    bool empty() const noexcept { return m_keyframes.empty(); }
    size_t size() const noexcept { return m_keyframes.size(); }
    void clear() noexcept { m_keyframes.clear(); }

    void sortKeyframes();

    std::pair<AnimationChannel, AnimationChannel> splitAt(core::TimelineTime splitTime) const;

private:
    std::string m_propertyName;
    double m_defaultValue{0.0};
    std::vector<Keyframe> m_keyframes;
};

} // namespace catchim::editor
