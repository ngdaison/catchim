#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>
#include <cstdint>

namespace catchim::render {

struct SpanStats {
    std::vector<double> samples;
};

struct CounterStats {
    int64_t total{0};
    int64_t frames{0};
};

struct SpanSummaryRow {
    std::string name;
    size_t count{0};
    double meanMs{0.0};
    double p50Ms{0.0};
    double p95Ms{0.0};
    double maxMs{0.0};
};

struct CounterSummaryRow {
    std::string name;
    double perFrame{0.0};
    int64_t total{0};
    int64_t frames{0};
};

class RenderPerformanceProfiler {
public:
    static constexpr int32_t DEFAULT_FLUSH_EVERY_FRAMES = 60;

    static RenderPerformanceProfiler& instance();

    bool isEnabled() const noexcept { return enabled_; }
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }

    void recordSpan(const std::string& name, double durationMs);
    void incrementCounter(const std::string& name, int64_t by = 1);

    template<typename Func>
    auto measureSpan(const std::string& name, Func&& func) {
        if (!enabled_) {
            return func();
        }
        auto start = std::chrono::high_resolution_clock::now();
        if constexpr (std::is_void_v<std::invoke_result_t<Func>>) {
            func();
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            recordSpan(name, ms);
        } else {
            auto result = func();
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            recordSpan(name, ms);
            return result;
        }
    }

    void onFrameComplete();
    void flush();
    void reset();

    int32_t framesSinceFlush() const noexcept { return framesSinceFlush_; }
    int32_t flushInterval() const noexcept { return flushInterval_; }
    void setFlushInterval(int32_t frames) noexcept { flushInterval_ = frames; }

    std::vector<SpanSummaryRow> getSpanSummaries() const;
    std::vector<CounterSummaryRow> getCounterSummaries() const;

private:
    RenderPerformanceProfiler() = default;

    bool enabled_{false};
    int32_t flushInterval_{DEFAULT_FLUSH_EVERY_FRAMES};
    int32_t framesSinceFlush_{0};

    std::unordered_map<std::string, SpanStats> spans_;
    std::unordered_map<std::string, CounterStats> counters_;
    std::unordered_map<std::string, int64_t> pendingCountersThisFrame_;
};

} // namespace catchim::render
