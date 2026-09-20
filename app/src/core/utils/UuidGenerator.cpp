#include "UuidGenerator.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace catchim::core {

std::string UuidGenerator::generateUUID() {
    thread_local std::random_device rd;
    thread_local std::mt19937_64 gen(rd());
    thread_local std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

    uint32_t data[4];
    for (int i = 0; i < 4; ++i) {
        data[i] = dis(gen);
    }

    uint8_t bytes[16];
    for (int i = 0; i < 4; ++i) {
        bytes[i * 4 + 0] = static_cast<uint8_t>((data[i] >> 24) & 0xFF);
        bytes[i * 4 + 1] = static_cast<uint8_t>((data[i] >> 16) & 0xFF);
        bytes[i * 4 + 2] = static_cast<uint8_t>((data[i] >> 8) & 0xFF);
        bytes[i * 4 + 3] = static_cast<uint8_t>(data[i] & 0xFF);
    }

    // Set version 4 (0100) in byte 6
    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    // Set variant 1 (10xx) in byte 8
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            oss << '-';
        }
        oss << std::setw(2) << static_cast<int>(bytes[i]);
    }
    return oss.str();
}

bool UuidGenerator::isValidUUID(const std::string& uuid) noexcept {
    if (uuid.size() != 36) {
        return false;
    }

    for (size_t i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (uuid[i] != '-') {
                return false;
            }
        } else {
            const char c = uuid[i];
            if (!std::isxdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
    }

    // Version must be 4
    if (uuid[14] != '4') {
        return false;
    }

    // Variant must be 8, 9, a, b (case insensitive)
    const char variantChar = static_cast<char>(std::tolower(static_cast<unsigned char>(uuid[19])));
    if (variantChar != '8' && variantChar != '9' && variantChar != 'a' && variantChar != 'b') {
        return false;
    }

    return true;
}

} // namespace catchim::core
