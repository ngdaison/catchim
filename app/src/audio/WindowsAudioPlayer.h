#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <atomic>
#include <mutex>

namespace catchim::audio {

class WindowsAudioPlayer {
public:
    WindowsAudioPlayer();
    ~WindowsAudioPlayer();

    bool init(int sampleRate = 48000, int channels = 2);
    void shutdown();

    // Write interleaved float PCM samples (-1.0f to 1.0f)
    void writeSamples(const float* floatSamples, size_t sampleCount);

    // Immediately stop playing buffers (called on pause or seek)
    void reset();

    void setVolume(float volume) { volume_ = volume; }
    float getVolume() const { return volume_; }

    bool isInitialized() const { return initialized_; }

    // Milliseconds of audio currently queued in the hardware buffer pool
    int queuedMilliseconds() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::atomic<float> volume_{1.0f};
    std::atomic<bool> initialized_{false};
    int sampleRate_{48000};
    int channels_{2};
};

} // namespace catchim::audio
