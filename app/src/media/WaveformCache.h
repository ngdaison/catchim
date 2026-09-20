#pragma once

#include "media/waveform/WaveformSummaryEngine.h"
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace catchim::media {

class WaveformCache {
public:
    static WaveformCache& instance();

    bool has(const std::string& sourceKey) const;
    std::optional<SourceWaveformSummary> get(const std::string& sourceKey) const;
    void put(const std::string& sourceKey, SourceWaveformSummary summary);
    void clearSource(const std::string& sourceKey);
    void clearAll();
    size_t size() const;

private:
    WaveformCache() = default;

    mutable std::mutex mutex_;
    std::unordered_map<std::string, SourceWaveformSummary> cache_;
};

} // namespace catchim::media
