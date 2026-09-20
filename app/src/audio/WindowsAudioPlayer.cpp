#include "WindowsAudioPlayer.h"
#include <algorithm>
#include <vector>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace catchim::audio {

struct BufferSlot {
#if defined(_WIN32)
    WAVEHDR hdr{};
#endif
    std::vector<int16_t> pcm;
    bool inFlight{false};
};

struct WindowsAudioPlayer::Impl {
#if defined(_WIN32)
    HWAVEOUT hWaveOut{nullptr};
#endif
    std::vector<BufferSlot> slots;
    size_t nextSlotIndex{0};
    mutable std::mutex mutex;
};

WindowsAudioPlayer::WindowsAudioPlayer()
    : impl_(std::make_unique<Impl>())
{
}

WindowsAudioPlayer::~WindowsAudioPlayer() {
    shutdown();
}

bool WindowsAudioPlayer::init(int sampleRate, int channels) {
    if (initialized_) return true;

    sampleRate_ = sampleRate;
    channels_ = channels;

#if defined(_WIN32)
    WAVEFORMATEX wfx{};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = static_cast<WORD>(channels_);
    wfx.nSamplesPerSec = static_cast<DWORD>(sampleRate_);
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    std::lock_guard<std::mutex> lock(impl_->mutex);
    MMRESULT res = waveOutOpen(
        &impl_->hWaveOut,
        WAVE_MAPPER,
        &wfx,
        0,
        0,
        CALLBACK_NULL
    );

    if (res != MMSYSERR_NOERROR || !impl_->hWaveOut) {
        return false;
    }

    // Allocate 8 buffer slots
    const size_t numSlots = 8;
    impl_->slots.resize(numSlots);
    for (auto& slot : impl_->slots) {
        slot.inFlight = false;
        std::memset(&slot.hdr, 0, sizeof(slot.hdr));
    }
    impl_->nextSlotIndex = 0;
    initialized_ = true;
    return true;
#else
    return false;
#endif
}

void WindowsAudioPlayer::shutdown() {
    if (!initialized_) return;
    reset();

#if defined(_WIN32)
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->hWaveOut) {
        waveOutClose(impl_->hWaveOut);
        impl_->hWaveOut = nullptr;
    }
    impl_->slots.clear();
#endif
    initialized_ = false;
}

void WindowsAudioPlayer::reset() {
    if (!initialized_) return;

#if defined(_WIN32)
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->hWaveOut) return;

    // Reset marks all pending buffers as WHDR_DONE
    waveOutReset(impl_->hWaveOut);

    for (auto& slot : impl_->slots) {
        if (slot.inFlight) {
            if (slot.hdr.dwFlags & WHDR_PREPARED) {
                waveOutUnprepareHeader(impl_->hWaveOut, &slot.hdr, sizeof(WAVEHDR));
            }
            slot.inFlight = false;
        }
    }
#endif
}

int WindowsAudioPlayer::queuedMilliseconds() const {
    if (!initialized_) return 0;

#if defined(_WIN32)
    std::lock_guard<std::mutex> lock(impl_->mutex);
    size_t inFlightBytes = 0;
    for (const auto& slot : impl_->slots) {
        if (slot.inFlight && !(slot.hdr.dwFlags & WHDR_DONE)) {
            inFlightBytes += slot.pcm.size() * sizeof(int16_t);
        }
    }
    size_t bytesPerMs = (sampleRate_ * channels_ * sizeof(int16_t)) / 1000;
    if (bytesPerMs == 0) return 0;
    return static_cast<int>(inFlightBytes / bytesPerMs);
#else
    return 0;
#endif
}

void WindowsAudioPlayer::writeSamples(const float* floatSamples, size_t sampleCount) {
    if (!initialized_ || !floatSamples || sampleCount == 0) return;

#if defined(_WIN32)
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->hWaveOut) return;

    // Cleanup any finished slots
    for (auto& slot : impl_->slots) {
        if (slot.inFlight && (slot.hdr.dwFlags & WHDR_DONE)) {
            waveOutUnprepareHeader(impl_->hWaveOut, &slot.hdr, sizeof(WAVEHDR));
            slot.inFlight = false;
        }
    }

    // Find an available slot
    BufferSlot* targetSlot = nullptr;
    for (size_t i = 0; i < impl_->slots.size(); ++i) {
        size_t idx = (impl_->nextSlotIndex + i) % impl_->slots.size();
        if (!impl_->slots[idx].inFlight) {
            targetSlot = &impl_->slots[idx];
            impl_->nextSlotIndex = (idx + 1) % impl_->slots.size();
            break;
        }
    }

    if (!targetSlot) {
        // All buffers currently queued in hardware
        return;
    }

    // Convert float to int16 with volume scaling
    float vol = volume_.load();
    targetSlot->pcm.resize(sampleCount);
    for (size_t i = 0; i < sampleCount; ++i) {
        float val = floatSamples[i] * vol;
        val = std::clamp(val, -1.0f, 1.0f);
        targetSlot->pcm[i] = static_cast<int16_t>(val * 32767.0f);
    }

    std::memset(&targetSlot->hdr, 0, sizeof(WAVEHDR));
    targetSlot->hdr.lpData = reinterpret_cast<LPSTR>(targetSlot->pcm.data());
    targetSlot->hdr.dwBufferLength = static_cast<DWORD>(targetSlot->pcm.size() * sizeof(int16_t));

    MMRESULT prepRes = waveOutPrepareHeader(impl_->hWaveOut, &targetSlot->hdr, sizeof(WAVEHDR));
    if (prepRes == MMSYSERR_NOERROR) {
        MMRESULT writeRes = waveOutWrite(impl_->hWaveOut, &targetSlot->hdr, sizeof(WAVEHDR));
        if (writeRes == MMSYSERR_NOERROR) {
            targetSlot->inFlight = true;
        } else {
            waveOutUnprepareHeader(impl_->hWaveOut, &targetSlot->hdr, sizeof(WAVEHDR));
        }
    }
#endif
}

} // namespace catchim::audio
