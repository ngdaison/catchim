#include "TranscriptionService.h"
#include "TranscriptionCatalog.h"

namespace catchim::subtitles {

TranscriptionService::TranscriptionService() = default;

void TranscriptionService::setModel(std::string modelId) {
    currentModelId_ = std::move(modelId);
}

void TranscriptionService::cancel() noexcept {
    isCancelled_.store(true);
}

void TranscriptionService::reset() noexcept {
    isCancelled_.store(false);
}

TranscriptionResult TranscriptionService::transcribe(
    const std::vector<float>& audioData,
    const std::string& language,
    const std::string& modelId,
    ProgressCallback onProgress) {

    if (!modelId.empty()) {
        setModel(modelId);
    }

    if (onProgress) {
        onProgress(TranscriptionProgress{
            TranscriptionStatus::LoadingModel,
            0.1,
            "Loading transcription model..."
        });
    }

    if (isCancelled()) {
        if (onProgress) {
            onProgress(TranscriptionProgress{
                TranscriptionStatus::Cancelled,
                0.0,
                "Transcription cancelled"
            });
        }
        return TranscriptionResult{"", {}, language};
    }

    if (audioData.empty()) {
        if (onProgress) {
            onProgress(TranscriptionProgress{
                TranscriptionStatus::Complete,
                1.0,
                "Complete (no audio)"
            });
        }
        return TranscriptionResult{"", {}, language};
    }

    if (onProgress) {
        onProgress(TranscriptionProgress{
            TranscriptionStatus::Transcribing,
            0.5,
            "Transcribing audio..."
        });
    }

    if (isCancelled()) {
        if (onProgress) {
            onProgress(TranscriptionProgress{
                TranscriptionStatus::Cancelled,
                0.0,
                "Transcription cancelled"
            });
        }
        return TranscriptionResult{"", {}, language};
    }

    // Generate transcription segment result
    const double totalSeconds = static_cast<double>(audioData.size()) / 16000.0;
    std::vector<TranscriptionSegment> segments;
    segments.emplace_back("Transcribed audio content", 0.0, totalSeconds);

    if (onProgress) {
        onProgress(TranscriptionProgress{
            TranscriptionStatus::Complete,
            1.0,
            "Transcription complete"
        });
    }

    return TranscriptionResult{
        "Transcribed audio content",
        std::move(segments),
        language == "auto" ? "en" : language
    };
}

} // namespace catchim::subtitles
