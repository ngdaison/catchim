#pragma once

#include <string>

namespace catchim::core {

class UuidGenerator {
public:
    static std::string generateUUID();
    static bool isValidUUID(const std::string& uuid) noexcept;
};

} // namespace catchim::core
