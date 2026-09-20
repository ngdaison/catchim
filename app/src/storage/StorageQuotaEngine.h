#pragma once

#include <string>
#include <cstdint>
#include <optional>

namespace catchim::storage {

constexpr uint64_t kStorageHeadroomReserveBytes = 50ULL * 1024ULL * 1024ULL; // 50MB

enum class StorageCapacityStatus {
    EnoughSpace,
    InsufficientSpace,
    EstimateUnavailable
};

struct StorageCapacityResult {
    bool canStore{true};
    StorageCapacityStatus status{StorageCapacityStatus::EstimateUnavailable};
    uint64_t availableBytes{0};
    uint64_t headroomBytes{0};
};

class StorageQuotaEngine {
public:
    // Formats byte count into human-readable representation: e.g. "12.5 MB", "1.2 GB"
    static std::string formatStorageBytes(uint64_t bytes);

    // Evaluates whether required bytes can fit into available storage with reserve headroom
    static StorageCapacityResult evaluateStorageCapacity(
        uint64_t requiredBytes,
        std::optional<uint64_t> quotaBytes = std::nullopt,
        std::optional<uint64_t> usageBytes = std::nullopt
    ) noexcept;
};

} // namespace catchim::storage
