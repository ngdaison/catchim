#pragma once

#include "TranscriptionEngine.h"
#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace catchim::subtitles {

enum class TranscriptionStatus {
    Idle,
    LoadingModel,
    Transcribing,
    Complete,
    Error,
    Cancelled
};

struct TranscriptionProgress {
    TranscriptionStatus status{TranscriptionStatus::Idle};
    double progress{0.0};
    std::string message;
};

struct TranscriptionResult {
    std::string text;
    std::vector<TranscriptionSegment> segments;
    std::string language;
};

class TranscriptionService {
public:
    using ProgressCallback = std::function<void(const TranscriptionProgress&)>;

    TranscriptionService();
    ~TranscriptionService() = default;

    void setModel(std::string modelId);
    const std::string& currentModel() const noexcept { return currentModelId_; }

    TranscriptionResult transcribe(
        const std::vector<float>& audioData,
        const std::string& language = "auto",
        const std::string& modelId = "whisper-small",
        ProgressCallback onProgress = nullptr);

    void cancel() noexcept;
    bool isCancelled() const noexcept { return isCancelled_.load(); }
    void reset() noexcept;

private:
    std::string currentModelId_{"whisper-small"};
    std::atomic<bool> isCancelled_{false};
};

} // namespace catchim::subtitles
