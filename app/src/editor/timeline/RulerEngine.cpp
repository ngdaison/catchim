#include "editor/timeline/RulerEngine.h"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace catchim::editor {

namespace {

constexpr int LABEL_FRAME_INTERVALS[] = {2, 3, 5, 10, 15};
constexpr int TICK_FRAME_INTERVALS[] = {1, 2, 3, 5, 10, 15};
constexpr int SECOND_MULTIPLIERS[] = {1, 2, 3, 5, 10, 15, 30, 60, 120, 300, 600, 900, 1800, 3600};

template <size_t N>
double findOptimalInterval(
    double pixelsPerFrame,
    double pixelsPerSecond,
    double fps,
    double minSpacingPx,
    const int (&frameIntervals)[N]
) {
    for (int frameInterval : frameIntervals) {
        double pixelSpacing = pixelsPerFrame * frameInterval;
        if (pixelSpacing >= minSpacingPx) {
            return static_cast<double>(frameInterval) / fps;
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

double ensureTickDividesLabel(
    double tickIntervalSeconds,
    double labelIntervalSeconds,
    double pixelsPerFrame,
    double pixelsPerSecond,
    double fps
) {
    long long labelFrames = static_cast<long long>(std::round(labelIntervalSeconds * fps));
    long long tickFrames = static_cast<long long>(std::round(tickIntervalSeconds * fps));

    if (tickFrames > 0 && labelFrames % tickFrames == 0) {
        return tickIntervalSeconds;
    }

    for (int candidateFrames : TICK_FRAME_INTERVALS) {
        if (labelFrames % candidateFrames == 0) {
            double candidateSpacing = pixelsPerFrame * candidateFrames;
            if (candidateSpacing >= RulerEngine::MIN_TICK_SPACING_PX) {
                return static_cast<double>(candidateFrames) / fps;
            }
        }
    }

    for (int candidateSeconds : SECOND_MULTIPLIERS) {
        double ratio = labelIntervalSeconds / static_cast<double>(candidateSeconds);
        if (std::abs(ratio - std::round(ratio)) < 0.0001) {
            double candidateSpacing = pixelsPerSecond * candidateSeconds;
            if (candidateSpacing >= RulerEngine::MIN_TICK_SPACING_PX) {
                return static_cast<double>(candidateSeconds);
            }
        }
    }

    return labelIntervalSeconds;
}

bool isSecondBoundary(double timeInSeconds) {
    constexpr double epsilon = 0.0001;
    double remainder = std::fmod(timeInSeconds, 1.0);
    if (remainder < 0.0) remainder += 1.0;
    return remainder < epsilon || remainder > 1.0 - epsilon;
}

std::string formatTimestamp(double timeInSeconds) {
    long long totalSeconds = static_cast<long long>(std::round(timeInSeconds));
    long long hours = totalSeconds / 3600;
    long long minutes = (totalSeconds % 3600) / 60;
    long long seconds = totalSeconds % 60;

    std::ostringstream ss;
    if (hours > 0) {
        ss << hours << ":"
           << (minutes < 10 ? "0" : "") << minutes << ":"
           << (seconds < 10 ? "0" : "") << seconds;
    } else {
        ss << (minutes < 10 ? "0" : "") << minutes << ":"
           << (seconds < 10 ? "0" : "") << seconds;
    }
    return ss.str();
}

} // namespace

RulerConfig RulerEngine::getRulerConfig(double zoomLevel, double fps) {
    if (fps <= 0.0) fps = 30.0;
    if (zoomLevel <= 0.0001) zoomLevel = 1.0;

    double pixelsPerSecond = BASE_PIXELS_PER_SECOND * zoomLevel;
    double pixelsPerFrame = pixelsPerSecond / fps;

    double labelIntervalSeconds = findOptimalInterval(
        pixelsPerFrame,
        pixelsPerSecond,
        fps,
        MIN_LABEL_SPACING_PX,
        LABEL_FRAME_INTERVALS
    );

    double rawTickIntervalSeconds = findOptimalInterval(
        pixelsPerFrame,
        pixelsPerSecond,
        fps,
        MIN_TICK_SPACING_PX,
        TICK_FRAME_INTERVALS
    );

    double tickIntervalSeconds = ensureTickDividesLabel(
        rawTickIntervalSeconds,
        labelIntervalSeconds,
        pixelsPerFrame,
        pixelsPerSecond,
        fps
    );

    return {labelIntervalSeconds, tickIntervalSeconds};
}

bool RulerEngine::shouldShowLabel(double timeInSeconds, double labelIntervalSeconds) {
    if (labelIntervalSeconds <= 0.0) return true;
    constexpr double epsilon = 0.0001;
    double remainder = std::fmod(timeInSeconds, labelIntervalSeconds);
    if (remainder < 0.0) remainder += labelIntervalSeconds;
    return remainder < epsilon || remainder > (labelIntervalSeconds - epsilon);
}

std::string RulerEngine::formatRulerLabel(double timeInSeconds, double fps) {
    if (timeInSeconds < 0.0) timeInSeconds = 0.0;
    if (isSecondBoundary(timeInSeconds)) {
        return formatTimestamp(timeInSeconds);
    }
    double fractionalPart = std::fmod(timeInSeconds, 1.0);
    if (fractionalPart < 0.0) fractionalPart += 1.0;
    int frameWithinSecond = static_cast<int>(std::round(fractionalPart * fps));
    return std::to_string(frameWithinSecond) + "f";
}

std::vector<RulerTick> RulerEngine::generateRulerTicks(
    core::TimelineTime startTime,
    core::TimelineTime endTime,
    double zoomLevel,
    double fps,
    double scrollOffsetX
) {
    std::vector<RulerTick> ticks;
    if (endTime < startTime || fps <= 0.0) return ticks;

    RulerConfig config = getRulerConfig(zoomLevel, fps);
    if (config.tickIntervalSeconds <= 0.0) return ticks;

    double startSec = startTime.toSeconds();
    double endSec = endTime.toSeconds();

    long long firstTickIndex = static_cast<long long>(std::floor(startSec / config.tickIntervalSeconds));
    if (firstTickIndex < 0) firstTickIndex = 0;

    double pixelsPerSecond = BASE_PIXELS_PER_SECOND * zoomLevel;

    for (long long idx = firstTickIndex;; ++idx) {
        double tickSec = idx * config.tickIntervalSeconds;
        if (tickSec > endSec + 0.0001) break;

        RulerTick tick;
        tick.time = core::TimelineTime::fromSeconds(tickSec);
        tick.pixelX = (tickSec * pixelsPerSecond) - scrollOffsetX;
        tick.isMajor = shouldShowLabel(tickSec, config.labelIntervalSeconds);
        if (tick.isMajor) {
            tick.label = formatRulerLabel(tickSec, fps);
        }
        ticks.push_back(tick);
    }

    return ticks;
}

} // namespace catchim::editor
