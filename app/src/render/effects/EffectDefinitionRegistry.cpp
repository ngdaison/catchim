#include "EffectDefinitionRegistry.h"
#include "core/utils/UuidGenerator.h"
#include <cmath>
#include <algorithm>

namespace catchim::render {

EffectDefinitionRegistry::EffectDefinitionRegistry() {
    registerBuiltinEffects();
}

EffectDefinitionRegistry& EffectDefinitionRegistry::instance() {
    static EffectDefinitionRegistry s_instance;
    return s_instance;
}

void EffectDefinitionRegistry::registerDefinition(EffectDefinition def) {
    definitions_[def.type] = def;
    definitionsList_.push_back(std::move(def));
}

const EffectDefinition* EffectDefinitionRegistry::getDefinition(const std::string& type) const noexcept {
    auto it = definitions_.find(type);
    if (it != definitions_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool EffectDefinitionRegistry::hasDefinition(const std::string& type) const noexcept {
    return definitions_.find(type) != definitions_.end();
}

std::vector<std::string> EffectDefinitionRegistry::getRegisteredTypes() const {
    std::vector<std::string> types;
    types.reserve(definitions_.size());
    for (const auto& [type, _] : definitions_) {
        types.push_back(type);
    }
    return types;
}

const std::vector<EffectDefinition>& EffectDefinitionRegistry::definitions() {
    return instance().definitionsList_;
}

const EffectDefinition* EffectDefinitionRegistry::findDefinition(const std::string& type) noexcept {
    return instance().getDefinition(type);
}

double EffectDefinitionRegistry::intensityToSigma(
    double intensity,
    double resolution,
    double reference
) noexcept {
    if (reference <= 0.0) return 0.0;
    return (intensity / INTENSITY_TO_SIGMA_DIVISOR) * (resolution / reference);
}

std::vector<EffectPass> EffectDefinitionRegistry::buildGaussianBlurPasses(
    double sigmaX,
    double sigmaY
) {
    if (sigmaY < 0.0) {
        sigmaY = sigmaX;
    }
    return BlurEffect::buildGaussianBlurPasses(
        static_cast<float>(sigmaX),
        static_cast<float>(sigmaY)
    );
}

editor::EffectInstance EffectDefinitionRegistry::buildDefaultEffectInstance(const std::string& effectType) {
    const auto* def = instance().getDefinition(effectType);
    editor::EffectInstance inst;
    inst.id = core::UuidGenerator::generateUUID();
    inst.type = effectType;
    inst.enabled = true;

    if (def != nullptr) {
        for (const auto& [k, v] : def->defaultParams) {
            inst.params[k] = v;
        }
    }
    return inst;
}

std::vector<EffectPass> EffectDefinitionRegistry::resolveEffectPasses(
    const editor::EffectInstance& instance,
    double width,
    double height
) {
    if (!instance.enabled) {
        return {};
    }

    if (instance.type == "blur") {
        double intensity = 15.0;
        if (instance.params.contains("intensity") && instance.params["intensity"].is_number()) {
            intensity = instance.params["intensity"].get<double>();
        }
        double sigmaX = intensityToSigma(intensity, width, 1920.0);
        double sigmaY = intensityToSigma(intensity, height, 1080.0);
        return buildGaussianBlurPasses(sigmaX, sigmaY);
    }

    return {};
}

void EffectDefinitionRegistry::registerBuiltinEffects() {
    EffectDefinition blurDef;
    blurDef.type = "blur";
    blurDef.name = "Blur";
    blurDef.keywords = {"blur", "soft", "defocus"};
    blurDef.parameters = {
        EffectDefinitionParam{
            .id = "intensity",
            .label = "Intensity",
            .type = "number",
            .defaultValue = 15.0,
            .min = 0.0,
            .max = 100.0,
            .step = 1.0,
        }
    };
    blurDef.defaultParams["intensity"] = 15.0;
    registerDefinition(std::move(blurDef));
}

} // namespace catchim::render
