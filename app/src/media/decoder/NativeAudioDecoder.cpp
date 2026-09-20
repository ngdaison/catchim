#include "NativeAudioDecoder.h"
#include <algorithm>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#endif

namespace catchim::media {

NativeAudioDecoder& NativeAudioDecoder::instance() {
    static NativeAudioDecoder s_instance;
    return s_instance;
}

NativeAudioDecoder::NativeAudioDecoder() {
#if defined(_WIN32)
    MFStartup(MF_VERSION);
#endif
}

NativeAudioDecoder::~NativeAudioDecoder() {
    clearCache();
#if defined(_WIN32)
    MFShutdown();
#endif
}

void NativeAudioDecoder::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_.clear();
}

bool NativeAudioDecoder::getAudioSamples(
    const std::filesystem::path& path,
    std::vector<float>& outPcmInterleaved,
    int targetSampleRate,
    int targetChannels
) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string pathKey = path.string();
    auto it = cache_.find(pathKey);
    if (it != cache_.end()) {
        outPcmInterleaved = *it->second;
        return !outPcmInterleaved.empty();
    }

#if defined(_WIN32)
    IMFSourceReader* pReader = nullptr;
    IMFAttributes* pAttributes = nullptr;
    MFCreateAttributes(&pAttributes, 1);
    if (pAttributes) {
        pAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);
    }

    std::wstring wpath = path.wstring();
    HRESULT hr = MFCreateSourceReaderFromURL(wpath.c_str(), pAttributes, &pReader);
    if (pAttributes) pAttributes->Release();

    if (FAILED(hr) || !pReader) {
        return false;
    }

    // Deselect all streams, select only first audio stream
    pReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_ALL_STREAMS), FALSE);
    hr = pReader->SetStreamSelection(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), TRUE);
    if (FAILED(hr)) {
        pReader->Release();
        return false;
    }

    // Configure requested PCM format: 48kHz, float, stereo
    IMFMediaType* pPartialType = nullptr;
    MFCreateMediaType(&pPartialType);
    if (pPartialType) {
        pPartialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        pPartialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
        pPartialType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, static_cast<UINT32>(targetChannels));
        pPartialType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, static_cast<UINT32>(targetSampleRate));
        pPartialType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 32);
        pPartialType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, static_cast<UINT32>(targetChannels * 4));
        pPartialType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, static_cast<UINT32>(targetSampleRate * targetChannels * 4));

        hr = pReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr, pPartialType);
        pPartialType->Release();
    }

    if (FAILED(hr)) {
        pReader->Release();
        return false;
    }

    auto pcmBuf = std::make_shared<std::vector<float>>();
    pcmBuf->reserve(targetSampleRate * targetChannels * 60); // 1 minute pre-reserve

    while (true) {
        DWORD streamIndex = 0, flags = 0;
        LONGLONG timestamp = 0;
        IMFSample* pSample = nullptr;

        hr = pReader->ReadSample(
            static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM),
            0,
            &streamIndex,
            &flags,
            &timestamp,
            &pSample
        );

        if (FAILED(hr) || (flags & MF_SOURCE_READERF_ENDOFSTREAM)) {
            if (pSample) pSample->Release();
            break;
        }

        if (flags & MF_SOURCE_READERF_STREAMTICK) {
            if (pSample) pSample->Release();
            continue;
        }

        if (pSample) {
            IMFMediaBuffer* pBuffer = nullptr;
            hr = pSample->ConvertToContiguousBuffer(&pBuffer);
            if (SUCCEEDED(hr) && pBuffer) {
                BYTE* pData = nullptr;
                DWORD currentLength = 0;
                hr = pBuffer->Lock(&pData, nullptr, &currentLength);
                if (SUCCEEDED(hr) && pData && currentLength >= sizeof(float)) {
                    size_t floatCount = currentLength / sizeof(float);
                    const float* floatData = reinterpret_cast<const float*>(pData);
                    pcmBuf->insert(pcmBuf->end(), floatData, floatData + floatCount);
                    pBuffer->Unlock();
                }
                pBuffer->Release();
            }
            pSample->Release();
        }
    }

    pReader->Release();

    if (!pcmBuf->empty()) {
        cache_[pathKey] = pcmBuf;
        outPcmInterleaved = *pcmBuf;
        return true;
    }
#endif

    return false;
}

} // namespace catchim::media
