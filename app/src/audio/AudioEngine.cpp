#include "AudioEngine.h"
#include "core/logging/Logger.h"

namespace catchim::audio {

AudioEngine::AudioEngine(int32_t sampleRate)
    : sampleRate_(sampleRate)
    , mixer_(2, sampleRate)
{
}

AudioEngine::~AudioEngine() {
    shutdown();
}

bool AudioEngine::init() {
    LOG_INFO("AudioEngine initialized at {} Hz", sampleRate_);
    return true;
}

void AudioEngine::shutdown() {
    stop();
}

void AudioEngine::start() {
    isRunning_ = true;
}

void AudioEngine::stop() {
    isRunning_ = false;
}

} // namespace catchim::audio
