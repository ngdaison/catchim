#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace catchim::render {

struct MaskFeatures {
    bool hasPosition{true};
    bool hasRotation{true};
    std::string sizeMode{"none"}; // "none" | "uniform" | "width-height" | "height-only" | "width-only"
};

struct RegisteredMaskDefinition {
    std::string type;
    std::string name;
    MaskFeatures features;
    std::vector<std::string> paramKeys;
};

/**
 * @brief Registry and factory for all mask types (builtin shapes and freeform).
 * Corresponds to web/src/masks/registry.ts and web/src/masks/builtin/definitions/.
 */
class MaskRegistry {
public:
    static const std::vector<std::string>& getAllMaskTypes();
    static bool isRegistered(const std::string& type) noexcept;
    static MaskFeatures getFeatures(const std::string& type) noexcept;
    static std::string getName(const std::string& type);

    /**
     * @brief Builds default JSON parameters for the given mask type and element dimensions.
     */
    static nlohmann::json buildDefault(
        const std::string& type,
        double elementWidth = 1920.0,
        double elementHeight = 1080.0
    );

    /**
     * @brief Determines whether a mask is active and should be applied during rendering.
     * For example, text masks with empty content are inactive.
     */
    static bool isActive(const std::string& type, const nlohmann::json& params);
};

} // namespace catchim::render
