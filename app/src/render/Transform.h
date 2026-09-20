#pragma once

#include <string>

namespace catchim::render {

struct CropBox {
    double top{0.0};
    double bottom{0.0};
    double left{0.0};
    double right{0.0};

    bool hasCrop() const noexcept {
        return top > 0.0 || bottom > 0.0 || left > 0.0 || right > 0.0;
    }
};

struct Transform {
    double positionX{0.0};
    double positionY{0.0};
    double scaleX{1.0};
    double scaleY{1.0};
    double rotate{0.0}; // in degrees
    double opacity{1.0};
    double anchorX{0.5};
    double anchorY{0.5};
    bool flipX{false};
    bool flipY{false};
    std::string blendMode{"normal"};
    CropBox crop{};

    bool isDefault() const noexcept {
        return positionX == 0.0 && positionY == 0.0 &&
               scaleX == 1.0 && scaleY == 1.0 &&
               rotate == 0.0 && opacity == 1.0 &&
               anchorX == 0.5 && anchorY == 0.5 &&
               !flipX && !flipY && blendMode == "normal" &&
               !crop.hasCrop();
    }
};

} // namespace catchim::render
