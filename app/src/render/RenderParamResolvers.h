#pragma once

#include "Transform.h"
#include "editor/animation/AnimationChannel.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace catchim::render {

class RenderParamResolvers {
public:
    static bool isBlendMode(const std::string& mode) noexcept;

    static std::string readBlendModeFromParams(const nlohmann::json& params);

    static double readOpacityFromParams(const nlohmann::json& params) noexcept;

    static Transform buildTransformFromParams(const nlohmann::json& params);

    static Transform resolveTransformAtTime(
        const Transform& baseTransform,
        const std::vector<editor::AnimationChannel>& channels,
        core::TimelineTime localTime
    );
};

} // namespace catchim::render
