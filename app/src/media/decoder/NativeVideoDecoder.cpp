#include "NativeVideoDecoder.h"
#include <algorithm>
#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <propvarutil.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "propsys.lib")
#endif

#if defined(HAVE_QT6)
#include <QImage>
#include <QString>
#endif

namespace catchim::media {

struct NativeVideoDecoder::SourceReaderContext {
#if defined(_WIN32)
    IMFSourceReader* pReader{nullptr};
#endif
    int width{0};
    int height{0};
    LONGLONG currentSampleHns{-1};
    double lastTimestamp{-1.0};
    std::vector<uint8_t> lastFrame;
    bool isImage{false};

    ~SourceReaderContext() {
#if defined(_WIN32)
        if (pReader) {
            pReader->Release();
            pReader = nullptr;
        }
#endif
    }
};

NativeVideoDecoder& NativeVideoDecoder::instance() {
    static NativeVideoDecoder s_instance;
    return s_instance;
}

NativeVideoDecoder::NativeVideoDecoder() {
#if defined(_WIN32)
    MFStartup(MF_VERSION);
#endif
}

NativeVideoDecoder::~NativeVideoDecoder() {
    clearCache();
#if defined(_WIN32)
    MFShutdown();
#endif
}

void NativeVideoDecoder::clearCache() {
    std::lock_guard<std::mutex> lock(mutex_);
    readers_.clear();
}

bool NativeVideoDecoder::getFrame(
    const std::filesystem::path& path,
    double timestampSec,
    int& outWidth,
    int& outHeight,
    std::vector<uint8_t>& outRgba,
    const std::string& clipId
) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string pathKey = path.string();
    std::string ext = path.extension().string();
    for (char& c : ext) c = static_cast<char>(::tolower(c));

    std::string readerKey = clipId.empty() ? pathKey : (clipId + "@" + pathKey);

    // 1. Image formats check
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".webp") {
        auto it = readers_.find(readerKey);
        if (it != readers_.end() && it->second->isImage && !it->second->lastFrame.empty()) {
            outWidth = it->second->width;
            outHeight = it->second->height;
            outRgba = it->second->lastFrame;
            return true;
        }

#if defined(HAVE_QT6)
        QImage img(QString::fromStdString(pathKey));
        if (!img.isNull()) {
            QImage conv = img.convertToFormat(QImage::Format_RGBA8888);
            outWidth = conv.width();
            outHeight = conv.height();
            outRgba.resize(outWidth * outHeight * 4);
            std::memcpy(outRgba.data(), conv.bits(), outRgba.size());

            auto ctx = std::make_shared<SourceReaderContext>();
            ctx->width = outWidth;
            ctx->height = outHeight;
            ctx->isImage = true;
            ctx->lastFrame = outRgba;
            readers_[readerKey] = ctx;
            return true;
        }
#endif
        return false;
    }

#if defined(_WIN32)
    auto it = readers_.find(readerKey);
    std::shared_ptr<SourceReaderContext> ctx;
    if (it != readers_.end()) {
        ctx = it->second;
    } else {
        ctx = std::make_shared<SourceReaderContext>();

        IMFAttributes* pAttributes = nullptr;
        MFCreateAttributes(&pAttributes, 2);
        if (pAttributes) {
            pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);
            pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
        }

        std::wstring wpath = path.wstring();
        HRESULT hr = MFCreateSourceReaderFromURL(wpath.c_str(), pAttributes, &ctx->pReader);
        if (pAttributes) pAttributes->Release();

        if (FAILED(hr) || !ctx->pReader) {
            return false;
        }

        // Configure output to RGB32 (32-bit B,G,R,A)
        IMFMediaType* pMediaType = nullptr;
        MFCreateMediaType(&pMediaType);
        if (pMediaType) {
            pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
            ctx->pReader->SetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), nullptr, pMediaType);
            pMediaType->Release();
        }

        // Query decoded width & height
        IMFMediaType* pCurrentType = nullptr;
        if (SUCCEEDED(ctx->pReader->GetCurrentMediaType(static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &pCurrentType)) && pCurrentType) {
            UINT32 w = 0, h = 0;
            MFGetAttributeSize(pCurrentType, MF_MT_FRAME_SIZE, &w, &h);
            ctx->width = static_cast<int>(w);
            ctx->height = static_cast<int>(h);
            pCurrentType->Release();
        }

        if (ctx->width <= 0 || ctx->height <= 0) {
            ctx->width = 1920;
            ctx->height = 1080;
        }

        readers_[readerKey] = ctx;
    }

    if (!ctx->pReader) return false;

    // Fast check: if requested time matches current cached frame within a small tolerance, return cached frame
    if (ctx->lastTimestamp >= 0.0 && std::abs(ctx->lastTimestamp - timestampSec) < 0.012 && !ctx->lastFrame.empty()) {
        outWidth = ctx->width;
        outHeight = ctx->height;
        outRgba = ctx->lastFrame;
        return true;
    }

    LONGLONG targetHns = static_cast<LONGLONG>(std::max(0.0, timestampSec) * 10000000.0);

    // Determine if seek is required:
    // 1) First read (currentSampleHns < 0)
    // 2) Backward jump (> 250ms behind current sample)
    // 3) Forward jump (> 2.5s ahead of current sample)
    bool needSeek = false;
    if (ctx->currentSampleHns < 0) {
        needSeek = true;
    } else if (targetHns < ctx->currentSampleHns - 2500000) {
        needSeek = true;
    } else if (targetHns > ctx->currentSampleHns + 25000000) {
        needSeek = true;
    }

    if (needSeek) {
        PROPVARIANT var;
        PropVariantInit(&var);
        var.vt = VT_I8;
        var.hVal.QuadPart = targetHns;
        ctx->pReader->SetCurrentPosition(GUID_NULL, var);
        PropVariantClear(&var);
        ctx->currentSampleHns = -1;
    }

    // Read samples sequentially until sampleTimestamp reaches or passes targetHns (with tolerance ~15ms)
    const LONGLONG toleranceHns = 150000; // 15ms
    int safetyLoops = 120;

    while (safetyLoops-- > 0) {
        // If we already have a sample close enough to or beyond target, we are done
        if (ctx->currentSampleHns >= 0 && ctx->currentSampleHns >= targetHns - toleranceHns) {
            break;
        }

        DWORD streamIndex = 0, flags = 0;
        LONGLONG sampleTimestamp = 0;
        IMFSample* pSample = nullptr;

        HRESULT hr = ctx->pReader->ReadSample(
            static_cast<DWORD>(MF_SOURCE_READER_FIRST_VIDEO_STREAM),
            0,
            &streamIndex,
            &flags,
            &sampleTimestamp,
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
                if (SUCCEEDED(hr) && pData && currentLength >= static_cast<DWORD>(ctx->width * ctx->height * 4)) {
                    ctx->lastFrame.resize(ctx->width * ctx->height * 4);

                    // Convert BGRA to RGBA
                    const int pixelCount = ctx->width * ctx->height;
                    for (int i = 0; i < pixelCount; ++i) {
                        uint8_t b = pData[i * 4 + 0];
                        uint8_t g = pData[i * 4 + 1];
                        uint8_t r = pData[i * 4 + 2];
                        uint8_t a = pData[i * 4 + 3];
                        ctx->lastFrame[i * 4 + 0] = r;
                        ctx->lastFrame[i * 4 + 1] = g;
                        ctx->lastFrame[i * 4 + 2] = b;
                        ctx->lastFrame[i * 4 + 3] = (a == 0) ? 255 : a;
                    }

                    pBuffer->Unlock();
                    ctx->currentSampleHns = sampleTimestamp;
                    ctx->lastTimestamp = static_cast<double>(sampleTimestamp) / 10000000.0;
                }
                pBuffer->Release();
            }
            pSample->Release();

            if (sampleTimestamp >= targetHns - toleranceHns) {
                break;
            }
        }
    }

    if (!ctx->lastFrame.empty()) {
        outWidth = ctx->width;
        outHeight = ctx->height;
        outRgba = ctx->lastFrame;
        return true;
    }
#endif

    return false;
}

} // namespace catchim::media
