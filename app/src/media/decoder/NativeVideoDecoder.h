#pragma once

#include "core/ids/Ids.h"
#include "core/time/TimelineTime.h"
#include <filesystem>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

namespace catchim::media {

class NativeVideoDecoder {
public:
    static NativeVideoDecoder& instance();

    NativeVideoDecoder();
    ~NativeVideoDecoder();

    // Decode or retrieve frame from cache
    bool getFrame(
        const std::filesystem::path& path,
        double timestampSec,
        int& outWidth,
        int& outHeight,
        std::vector<uint8_t>& outRgba
    );

    void clearCache();

private:
    struct SourceReaderContext;
    std::unordered_map<std::string, std::shared_ptr<SourceReaderContext>> readers_;
    std::mutex mutex_;
};

} // namespace catchim::media
