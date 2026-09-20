#include "RulerIntervalEngine.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace catchim::editor {

RulerConfig RulerIntervalEngine::getRulerConfig(
    double zoomLevel,
    const core::FrameRate& fps
) noexcept {
    double fpsFloat = core::RationalFrameRateHelper::toFloat(fps);
    return getRulerConfig(zoomLevel, fpsFloat);
}

RulerConfig RulerIntervalEngine::getRulerConfig(
    double zoomLevel,
    double fpsFloat
) noexcept {
    if (fpsFloat <= 0.0) {
        fpsFloat = 30.0;
    }
    double pixelsPerSecond = BASE_TIMELINE_PIXELS_PER_SECOND * zoomLevel;
    double pixelsPerFrame = pixelsPerSecond / fpsFloat;

    double labelIntervalSeconds = findOptimalInterval(
        pixelsPerFrame,
        pixelsPerSecond,
        fpsFloat,
        MIN_LABEL_SPACING_PX,
        LABEL_FRAME_INTERVALS.data(),
        LABEL_FRAME_INTERVALS.size()
    );

    double rawTickIntervalSeconds = findOptimalInterval(
        pixelsPerFrame,
        pixelsPerSecond,
        fpsFloat,
        MIN_TICK_SPACING_PX,
        TICK_FRAME_INTERVALS.data(),
        TICK_FRAME_INTERVALS.size()
    );

    double tickIntervalSeconds = ensureTickDividesLabel(
        rawTickIntervalSeconds,
        labelIntervalSeconds,
        pixelsPerFrame,
        pixelsPerSecond,
        fpsFloat
    );

    return RulerConfig{
        .labelIntervalSeconds = labelIntervalSeconds,
        .tickIntervalSeconds = tickIntervalSeconds
    };
}

double RulerIntervalEngine::ensureTickDividesLabel(
    double tickIntervalSeconds,
    double labelIntervalSeconds,
    double pixelsPerFrame,
    double pixelsPerSecond,
    double fpsFloat
) noexcept {
    int64_t labelFrames = static_cast<int64_t>(std::round(labelIntervalSeconds * fpsFloat));
    int64_t tickFrames = static_cast<int64_t>(std::round(tickIntervalSeconds * fpsFloat));

    if (tickFrames > 0 && (labelFrames % tickFrames == 0)) {
        return tickIntervalSeconds;
    }

    for (int candidateFrames : TICK_FRAME_INTERVALS) {
        if (candidateFrames > 0 && (labelFrames % candidateFrames == 0)) {
            double candidateSpacing = pixelsPerFrame * candidateFrames;
            if (candidateSpacing >= MIN_TICK_SPACING_PX) {
                return static_cast<double>(candidateFrames) / fpsFloat;
            }
        }
    }

    for (int candidateSeconds : SECOND_MULTIPLIERS) {
        if (candidateSeconds > 0) {
            double ratio = labelIntervalSeconds / static_cast<double>(candidateSeconds);
            bool isDivisor = std::abs(ratio - std::round(ratio)) < 0.0001;
            if (isDivisor) {
                double candidateSpacing = pixelsPerSecond * candidateSeconds;
                if (candidateSpacing >= MIN_TICK_SPACING_PX) {
                    return static_cast<double>(candidateSeconds);
                }
            }
        }
    }

    return labelIntervalSeconds;
}

double RulerIntervalEngine::findOptimalInterval(
    double pixelsPerFrame,
    double pixelsPerSecond,
    double fpsFloat,
    double minSpacingPx,
    const int* frameIntervals,
    size_t frameIntervalsCount
) noexcept {
    for (size_t i = 0; i < frameIntervalsCount; ++i) {
        int fi = frameIntervals[i];
        double pixelSpacing = pixelsPerFrame * fi;
        if (pixelSpacing >= minSpacingPx) {
            return static_cast<double>(fi) / fpsFloat;
        }
    }

    for (int secondMultiplier : SECOND_MULTIPLIERS) {
        double pixelSpacing = pixelsPerSecond * secondMultiplier;
        if (pixelSpacing >= minSpacingPx) {
            return static_cast<double>(secondMultiplier);
        }
    }

    return 60.0;
}

bool RulerIntervalEngine::shouldShowLabel(
    double timeInSeconds,
    double labelIntervalSeconds
) noexcept {
    if (labelIntervalSeconds <= 0.0) return false;
    double epsilon = 0.0001;
    double remainder = std::fmod(timeInSeconds, labelIntervalSeconds);
    return remainder < epsilon || remainder > (labelIntervalSeconds - epsilon);
}

std::string RulerIntervalEngine::formatRulerLabel(
    double timeInSeconds,
    const core::FrameRate& fps
) {
    double fpsFloat = core::RationalFrameRateHelper::toFloat(fps);
    return formatRulerLabel(timeInSeconds, fpsFloat);
}

std::string RulerIntervalEngine::formatRulerLabel(
    double timeInSeconds,
    double fpsFloat
) {
    if (isSecondBoundary(timeInSeconds)) {
        return formatTimestamp(timeInSeconds);
    }

    int frameWithinSecond = getFrameWithinSecond(timeInSeconds, fpsFloat);
    return std::to_string(frameWithinSecond) + "f";
}

bool RulerIntervalEngine::isSecondBoundary(double timeInSeconds) noexcept {
    double epsilon = 0.0001;
    double remainder = std::fmod(timeInSeconds, 1.0);
    return remainder < epsilon || remainder > (1.0 - epsilon);
}

int RulerIntervalEngine::getFrameWithinSecond(
    double timeInSeconds,
    double fpsFloat
) noexcept {
    double fractionalPart = std::fmod(timeInSeconds, 1.0);
    return static_cast<int>(std::round(fractionalPart * fpsFloat));
}

std::string RulerIntervalEngine::formatTimestamp(double timeInSeconds) {
    int64_t totalSeconds = static_cast<int64_t>(std::round(timeInSeconds));
    int64_t hours = totalSeconds / 3600;
    int64_t minutes = (totalSeconds % 3600) / 60;
    int64_t seconds = totalSeconds % 60;

    std::ostringstream ss;
    if (hours > 0) {
        ss << hours << ":";
        if (minutes < 10) ss << "0";
        ss << minutes << ":";
        if (seconds < 10) ss << "0";
        ss << seconds;
    } else {
        if (minutes < 10) ss << "0";
        ss << minutes << ":";
        if (seconds < 10) ss << "0";
        ss << seconds;
    }
    return ss.str();
}

} // namespace catchim::editor
