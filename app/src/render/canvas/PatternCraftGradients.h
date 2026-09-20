#pragma once

#include <optional>
#include <string>
#include <vector>

namespace catchim::render {

class PatternCraftGradients {
public:
    static const std::vector<std::string>& getGradients();
    static const std::vector<std::string>& getSolidColors();
    static std::optional<std::string> findGradientByIndex(size_t index);
    static bool isValidHexColor(const std::string& hex) noexcept;
};

} // namespace catchim::render
