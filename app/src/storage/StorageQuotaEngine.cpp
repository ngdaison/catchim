#include "storage/StorageQuotaEngine.h"
#include "core/math/MathExpressionEvaluator.h"
#include <array>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace catchim::storage {

static const std::array<const char*, 5> kByteUnits = {
    "B", "KB", "MB", "GB", "TB"
};

std::string StorageQuotaEngine::formatStorageBytes(uint64_t bytes) {
    if (bytes == 0) {
        return "0 B";
    }

    double value = static_cast<double>(bytes);
    size_t unitIndex = 0;

    while (value >= 1024.0 && unitIndex < kByteUnits.size() - 1) {
        value /= 1024.0;
        unitIndex++;
    }

    int precision = (value >= 10.0 || unitIndex == 0) ? 0 : 1;
    return core::MathExpressionEvaluator::formatNumberForDisplay(value, 0, precision) + " " + kByteUnits[unitIndex];
}

StorageCapacityResult StorageQuotaEngine::evaluateStorageCapacity(
    uint64_t requiredBytes,
    std::optional<uint64_t> quotaBytes,
    std::optional<uint64_t> usageBytes
) noexcept {
    if (!quotaBytes.has_value() || !usageBytes.has_value()) {
        return StorageCapacityResult{
            true,
            StorageCapacityStatus::EstimateUnavailable,
            0,
            0
        };
    }

    uint64_t quota = *quotaBytes;
    uint64_t usage = *usageBytes;

    uint64_t headroom = (quota > usage) ? (quota - usage) : 0ULL;
    uint64_t available = (headroom > kStorageHeadroomReserveBytes)
        ? (headroom - kStorageHeadroomReserveBytes)
        : 0ULL;

    bool canStore = (requiredBytes <= available);
    auto status = canStore
        ? StorageCapacityStatus::EnoughSpace
        : StorageCapacityStatus::InsufficientSpace;

    return StorageCapacityResult{
        canStore,
        status,
        available,
        headroom
    };
}

} // namespace catchim::storage
