#include "MediaProbe.h"
#include <algorithm>
#include <string>

namespace catchim::media {

namespace {
std::string toLower(std::string_view sv) {
    std::string s(sv);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}
} // namespace

MediaType MediaProbe::detectType(const std::filesystem::path& path) {
    std::string ext = toLower(path.extension().string());
    if (ext == ".mp4" || ext == ".mov" || ext == ".mkv" || ext == ".webm" ||
        ext == ".avi" || ext == ".m4v" || ext == ".wmv" || ext == ".flv") {
        return MediaType::Video;
    }
    if (ext == ".mp3" || ext == ".wav" || ext == ".aac" || ext == ".m4a" ||
        ext == ".flac" || ext == ".ogg" || ext == ".wma" || ext == ".opus") {
        return MediaType::Audio;
    }
    return MediaType::Image;
}

core::Result<std::shared_ptr<MediaAsset>> MediaProbe::probe(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || !std::filesystem::is_regular_file(path, ec)) {
        return core::Result<std::shared_ptr<MediaAsset>>(core::ErrorCode::FileNotFound, "Media file does not exist");
    }

    MediaType type = detectType(path);
    auto asset = std::make_shared<MediaAsset>(core::MediaId::generate(), path, type);

    // Default metadata probe (can be enhanced with FFprobe if available)
    if (type == MediaType::Video) {
        asset->setDimensions(1920, 1080);
        asset->setFps(core::FrameRate{30, 1});
        asset->setHasVideo(true);
        asset->setHasAudio(true);
        // Default estimate duration (e.g. 10s if not probed)
        asset->setDuration(core::TimelineTime::fromSeconds(10.0));
    } else if (type == MediaType::Audio) {
        asset->setHasAudio(true);
        asset->setHasVideo(false);
        asset->setDuration(core::TimelineTime::fromSeconds(30.0));
    } else if (type == MediaType::Image) {
        asset->setDimensions(1920, 1080);
        asset->setHasVideo(false);
        asset->setHasAudio(false);
        asset->setDuration(core::TimelineTime::fromSeconds(5.0)); // Default image clip duration
    }

    return core::Result<std::shared_ptr<MediaAsset>>(std::move(asset));
}

} // namespace catchim::media
