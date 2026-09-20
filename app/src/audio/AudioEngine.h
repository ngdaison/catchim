#pragma once

#include "AudioMixer.h"
#include <memory>
#include <atomic>

namespace catchim::audio {

class AudioEngine {
public:
    AudioEngine(int32_t sampleRate = 44100);
    ~AudioEngine();

    bool init();
    void shutdown();

    void start();
    void stop();

    AudioMixer& mixer() noexcept { return mixer_; }
    const AudioMixer& mixer() const noexcept { return mixer_; }

private:
    int32_t sampleRate_{44100};
    AudioMixer mixer_;
    std::atomic<bool> isRunning_{false};
};

} // namespace catchim::audio
