#pragma once
// Feature 228 -- mirrors web/src/media/upload-toast.ts
#include <string>
#include <vector>

namespace catchim::media {

class MediaUploadToastEngine {
public:
    static std::string getAssetLabel(int count);
    static std::string formatLoadingMessage(int filesCount);
    static std::string formatSuccessMessage(int uploadedCount, const std::vector<std::string>& assetNames = {});
    static std::string formatErrorMessage(int filesCount);
};

} // namespace catchim::media
