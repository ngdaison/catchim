// Feature 228 -- mirrors web/src/media/upload-toast.ts
#include "media/MediaUploadToastEngine.h"

namespace catchim::media {

std::string MediaUploadToastEngine::getAssetLabel(int count) {
    return (count == 1) ? "media asset" : "media assets";
}

std::string MediaUploadToastEngine::formatLoadingMessage(int filesCount) {
    return "Uploading " + getAssetLabel(filesCount) + "...";
}

std::string MediaUploadToastEngine::formatSuccessMessage(int uploadedCount, const std::vector<std::string>& assetNames) {
    if (uploadedCount == 1) {
        if (!assetNames.empty() && !assetNames[0].empty()) {
            return assetNames[0] + " has been uploaded";
        }
        return "1 media asset has been uploaded";
    }
    if (uploadedCount > 1) {
        return std::to_string(uploadedCount) + " media assets have been uploaded";
    }
    return "No media assets were uploaded";
}

std::string MediaUploadToastEngine::formatErrorMessage(int filesCount) {
    return "Failed to upload " + getAssetLabel(filesCount);
}

} // namespace catchim::media
