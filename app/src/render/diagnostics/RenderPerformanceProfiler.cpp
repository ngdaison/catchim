#include "render/diagnostics/RenderPerformanceProfiler.h"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace catchim::render {

RenderPerformanceProfiler& RenderPerformanceProfiler::instance() {
    static RenderPerformanceProfiler profiler;
    return profiler;
}

void RenderPerformanceProfiler::recordSpan(const std::string& name, double durationMs) {
    if (!enabled_) return;
    spans_[name].samples.push_back(durationMs);
}

void RenderPerformanceProfiler::incrementCounter(const std::string& name, int64_t by) {
    if (!enabled_) return;
    pendingCountersThisFrame_[name] += by;
}

void RenderPerformanceProfiler::onFrameComplete() {
    if (!enabled_) return;

    for (const auto& [name, count] : pendingCountersThisFrame_) {
        auto& stats = counters_[name];
        stats.total += count;
        stats.frames += 1;
    }
    pendingCountersThisFrame_.clear();

    framesSinceFlush_++;
    if (framesSinceFlush_ >= flushInterval_) {
        flush();
    }
}

std::vector<SpanSummaryRow> RenderPerformanceProfiler::getSpanSummaries() const {
    std::vector<SpanSummaryRow> rows;
    rows.reserve(spans_.size());

    for (const auto& [name, stats] : spans_) {
        if (stats.samples.empty()) continue;

        std::vector<double> sorted = stats.samples;
        std::sort(sorted.begin(), sorted.end());

        double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
        double count = static_cast<double>(sorted.size());
        double meanMs = sum / count;

        auto quantile = [&](double q) {
            size_t idx = std::min(sorted.size() - 1, static_cast<size_t>(std::floor(count * q)));
            return sorted[idx];
        };

        SpanSummaryRow row;
        row.name = name;
        row.count = sorted.size();
        row.meanMs = meanMs;
        row.p50Ms = quantile(0.5);
        row.p95Ms = quantile(0.95);
        row.maxMs = sorted.back();

        rows.push_back(row);
    }

    std::sort(rows.begin(), rows.end(), [](const SpanSummaryRow& a, const SpanSummaryRow& b) {
        return a.meanMs > b.meanMs;
    });

    return rows;
}

std::vector<CounterSummaryRow> RenderPerformanceProfiler::getCounterSummaries() const {
    std::vector<CounterSummaryRow> rows;
    rows.reserve(counters_.size());

    for (const auto& [name, stats] : counters_) {
        int64_t f = std::max(int64_t{1}, stats.frames);
        CounterSummaryRow row;
        row.name = name;
        row.perFrame = static_cast<double>(stats.total) / static_cast<double>(f);
        row.total = stats.total;
        row.frames = stats.frames;
        rows.push_back(row);
    }

    std::sort(rows.begin(), rows.end(), [](const CounterSummaryRow& a, const CounterSummaryRow& b) {
        return a.perFrame > b.perFrame;
    });

    return rows;
}

void RenderPerformanceProfiler::flush() {
    spans_.clear();
    counters_.clear();
    pendingCountersThisFrame_.clear();
    framesSinceFlush_ = 0;
}

void RenderPerformanceProfiler::reset() {
    flush();
}

} // namespace catchim::render
