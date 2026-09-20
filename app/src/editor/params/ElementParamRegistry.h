#pragma once

#include "editor/timeline/Clip.h"
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace catchim::editor {

struct BlendModeOption {
    std::string value;
    std::string label;

    bool operator==(const BlendModeOption& other) const noexcept = default;
};

struct ElementParamDefinition {
    std::string key;
    std::string label;
    std::string type; // "number", "select", "string", "boolean", "color"
    nlohmann::json defaultValue;
    std::optional<double> min{std::nullopt};
    std::optional<double> max{std::nullopt};
    std::optional<double> step{std::nullopt};

    bool operator==(const ElementParamDefinition& other) const = default;
};

class ElementParamRegistry {
public:
    // Returns the 17 standard CSS/Canvas blend modes supported in Catchim
    static const std::vector<BlendModeOption>& getBlendModeOptions() noexcept;

    // Returns built-in parameter definitions for a specific clip type
    static std::vector<ElementParamDefinition> getBuiltInElementParams(ClipType type);

    // Generates a complete JSON parameter map populated with default values for the given clip type
    static nlohmann::json buildDefaultParamValues(ClipType type);
};

} // namespace catchim::editor
