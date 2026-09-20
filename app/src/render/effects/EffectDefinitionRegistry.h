#pragma once

#include "render/effects/BlurEffect.h"
#include "editor/timeline/Clip.h"
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <utility>

namespace catchim::render {

struct EffectDefinitionParam {
    std::string id;
    std::string label;
    std::string type{"number"};
    double defaultValue{0.0};
    double min{0.0};
    double max{100.0};
    double step{1.0};
};

struct EffectDefinition {
    std::string type;
    std::string name;
    std::vector<std::string> keywords;
    std::vector<EffectDefinitionParam> parameters;
    std::map<std::string, double> defaultParams;
};

/**
 * @brief Registry for effect definitions, default instance builders, and multi-pass shader generators.
 * Corresponds to web/src/effects/ (types.ts, index.ts, registry.ts, definitions/blur.ts).
 */
class EffectDefinitionRegistry {
public:
    static inline const std::string GAUSSIAN_BLUR_SHADER = "gaussian-blur";
    static constexpr double INTENSITY_TO_SIGMA_DIVISOR = 5.0;

    EffectDefinitionRegistry();

    static EffectDefinitionRegistry& instance();

    void registerDefinition(EffectDefinition def);
    const EffectDefinition* getDefinition(const std::string& type) const noexcept;
    bool hasDefinition(const std::string& type) const noexcept;
    std::vector<std::string> getRegisteredTypes() const;

    static const std::vector<EffectDefinition>& definitions();
    static const EffectDefinition* findDefinition(const std::string& type) noexcept;

    static double intensityToSigma(
        double intensity,
        double resolution = 1920.0,
        double reference = 1920.0
    ) noexcept;

    static std::vector<EffectPass> buildGaussianBlurPasses(
        double sigmaX,
        double sigmaY = -1.0
    );

    static editor::EffectInstance buildDefaultEffectInstance(const std::string& effectType);

    static std::vector<EffectPass> resolveEffectPasses(
        const editor::EffectInstance& instance,
        double width = 1920.0,
        double height = 1080.0
    );

private:
    void registerBuiltinEffects();

    std::map<std::string, EffectDefinition> definitions_;
    std::vector<EffectDefinition> definitionsList_;
};

} // namespace catchim::render
