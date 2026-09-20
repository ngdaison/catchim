#pragma once

#include <filesystem>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace catchim::media {

class NativeAudioDecoder {
public:
    static NativeAudioDecoder& instance();

    NativeAudioDecoder();
    ~NativeAudioDecoder();

    // Decode or retrieve full audio stream as 48kHz stereo float PCM (-1.0f to 1.0f)
    bool getAudioSamples(
        const std::filesystem::path& path,
        std::vector<float>& outPcmInterleaved,
        int targetSampleRate = 48000,
        int targetChannels = 2
    );

    void clearCache();

private:
    std::unordered_map<std::string, std::shared_ptr<std::vector<float>>> cache_;
    std::mutex mutex_;
};

} // namespace catchim::media
