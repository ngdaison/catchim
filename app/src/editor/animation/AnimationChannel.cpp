#include "editor/animation/AnimationChannel.h"
#include "core/math/Bezier.h"
#include <algorithm>
#include <cmath>

namespace catchim::editor {

void AnimationChannel::sortKeyframes() {
    std::sort(m_keyframes.begin(), m_keyframes.end());
}

void AnimationChannel::addOrUpdateKeyframe(Keyframe kf) {
    for (auto& existing : m_keyframes) {
        if (existing.time == kf.time) {
            existing = kf;
            return;
        }
    }
    m_keyframes.push_back(std::move(kf));
    sortKeyframes();
}

bool AnimationChannel::removeKeyframeAt(core::TimelineTime time, core::TimelineTime threshold) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), [&](const Keyframe& kf) {
        return std::abs(kf.time.ticks() - time.ticks()) <= threshold.ticks();
    });

    if (it != m_keyframes.end()) {
        m_keyframes.erase(it);
        return true;
    }
    return false;
}

std::optional<Keyframe> AnimationChannel::findKeyframeAt(core::TimelineTime time, core::TimelineTime threshold) const {
    for (const auto& kf : m_keyframes) {
        if (std::abs(kf.time.ticks() - time.ticks()) <= threshold.ticks()) {
            return kf;
        }
    }
    return std::nullopt;
}

double AnimationChannel::getValueAt(core::TimelineTime time) const {
    if (m_keyframes.empty()) {
        return m_defaultValue;
    }

    if (m_keyframes.size() == 1) {
        return m_keyframes[0].value;
    }

    // Boundary conditions
    if (time <= m_keyframes.front().time) {
        return m_keyframes.front().value;
    }
    if (time >= m_keyframes.back().time) {
        return m_keyframes.back().value;
    }

    // Binary search for interval
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), time,
        [](const Keyframe& kf, core::TimelineTime t) {
            return kf.time < t;
        });

    if (it == m_keyframes.begin()) {
        return it->value;
    }

    const Keyframe& k2 = *it;
    const Keyframe& k1 = *(it - 1);

    int64_t segmentDurationTicks = k2.time.ticks() - k1.time.ticks();
    if (segmentDurationTicks <= 0) {
        return k1.value;
    }

    double progress = static_cast<double>(time.ticks() - k1.time.ticks()) / static_cast<double>(segmentDurationTicks);
    progress = std::clamp(progress, 0.0, 1.0);

    switch (k1.interpolation) {
        case KeyframeInterpolation::Hold:
            return k1.value;

        case KeyframeInterpolation::Linear:
            return k1.value + progress * (k2.value - k1.value);

        case KeyframeInterpolation::Bezier: {
            double solvedT = core::BezierSolver::solve(progress, k1.bezierX1, k1.bezierY1, k1.bezierX2, k1.bezierY2);
            return k1.value + solvedT * (k2.value - k1.value);
        }
    }

    return k1.value;
}

std::pair<AnimationChannel, AnimationChannel> AnimationChannel::splitAt(core::TimelineTime splitTime) const {
    AnimationChannel left(m_propertyName, m_defaultValue);
    AnimationChannel right(m_propertyName, m_defaultValue);

    if (m_keyframes.empty()) {
        return {left, right};
    }

    double boundaryValue = getValueAt(splitTime);

    // Left channel keys
    bool hasLeftBoundary = false;
    for (const auto& kf : m_keyframes) {
        if (kf.time < splitTime) {
            left.addOrUpdateKeyframe(kf);
        } else if (kf.time == splitTime) {
            left.addOrUpdateKeyframe(kf);
            hasLeftBoundary = true;
        }
    }
    if (!hasLeftBoundary) {
        Keyframe leftBoundary;
        leftBoundary.time = splitTime;
        leftBoundary.value = boundaryValue;
        leftBoundary.interpolation = KeyframeInterpolation::Linear;
        left.addOrUpdateKeyframe(leftBoundary);
    }

    // Right channel keys
    bool hasRightBoundary = false;
    for (const auto& kf : m_keyframes) {
        if (kf.time == splitTime) {
            Keyframe rk = kf;
            rk.time = core::TimelineTime(0);
            right.addOrUpdateKeyframe(rk);
            hasRightBoundary = true;
        } else if (kf.time > splitTime) {
            Keyframe rk = kf;
            rk.time = kf.time - splitTime;
            right.addOrUpdateKeyframe(rk);
        }
    }
    if (!hasRightBoundary) {
        Keyframe rightBoundary;
        rightBoundary.time = core::TimelineTime(0);
        rightBoundary.value = boundaryValue;
        rightBoundary.interpolation = KeyframeInterpolation::Linear;
        right.addOrUpdateKeyframe(rightBoundary);
    }

    left.sortKeyframes();
    right.sortKeyframes();
    return {left, right};
}

} // namespace catchim::editor
