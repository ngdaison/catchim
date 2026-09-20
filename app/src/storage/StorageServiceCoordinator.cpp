#include "StorageServiceCoordinator.h"
#include <algorithm>
#include <cctype>

namespace catchim::storage {

const StorageConfig& StorageServiceCoordinator::defaultConfig() noexcept {
    static const StorageConfig cfg;
    return cfg;
}

std::string StorageServiceCoordinator::getProjectMediaStoreName(
    std::string_view projectId,
    const StorageConfig& config
) {
    return config.mediaDb + "-" + std::string(projectId);
}

std::string StorageServiceCoordinator::getProjectMediaFilesFolder(
    std::string_view projectId
) {
    return "media-files-" + std::string(projectId);
}

std::vector<NormalizedBookmark> StorageServiceCoordinator::normalizeBookmarks(
    const nlohmann::json& raw
) {
    std::vector<NormalizedBookmark> result;
    if (!raw.is_array()) return result;

    for (const auto& item : raw) {
        if (item.is_number()) {
            // Integer ticks or fractional seconds
            int64_t ticks = item.get<int64_t>();
            result.push_back(NormalizedBookmark{
                .time = core::TimelineTime(ticks)
            });
        } else if (item.is_object()) {
            if (!item.contains("time") || !item["time"].is_number()) {
                continue;
            }
            NormalizedBookmark nb;
            nb.time = core::TimelineTime(item["time"].get<int64_t>());

            if (item.contains("note") && item["note"].is_string()) {
                nb.note = item["note"].get<std::string>();
            }
            if (item.contains("color") && item["color"].is_string()) {
                nb.color = item["color"].get<std::string>();
            }
            if (item.contains("duration") && item["duration"].is_number()) {
                nb.duration = core::TimelineTime(item["duration"].get<int64_t>());
            }
            result.push_back(std::move(nb));
        }
    }

    return result;
}

bool StorageServiceCoordinator::isQuotaExceededError(
    std::string_view errorMessage
) noexcept {
    std::string lower;
    lower.reserve(errorMessage.size());
    for (char c : errorMessage) {
        lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    return lower.find("quota") != std::string::npos ||
           lower.find("storage full") != std::string::npos ||
           lower.find("disk full") != std::string::npos ||
           lower.find("insufficient space") != std::string::npos;
}

} // namespace catchim::storage
