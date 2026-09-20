#pragma once

#include "render/HitTesting.h"
#include <nlohmann/json.hpp>
#include <string>
#include <optional>
#include <cmath>

namespace catchim::render {

struct Transform2D {
    double scaleX{1.0};
    double scaleY{1.0};
    double positionX{0.0};
    double positionY{0.0};
    double rotate{0.0}; // in degrees

    bool operator==(const Transform2D& other) const = default;
};

struct Matrix3x3 {
    double m[3][3]{
        {1.0, 0.0, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}
    };

    static Matrix3x3 identity() noexcept;
    static Matrix3x3 translation(double tx, double ty) noexcept;
    static Matrix3x3 rotation(double radians) noexcept;
    static Matrix3x3 scale(double sx, double sy) noexcept;
    static Matrix3x3 multiply(const Matrix3x3& a, const Matrix3x3& b) noexcept;
    static Matrix3x3 compose(double tx, double ty, double rotDegrees, double sx, double sy) noexcept;

    [[nodiscard]] double determinant() const noexcept;
    [[nodiscard]] std::optional<Matrix3x3> inverse() const noexcept;
    [[nodiscard]] Point2D transformPoint(const Point2D& p) const noexcept;
    [[nodiscard]] std::optional<Point2D> inverseTransformPoint(const Point2D& p) const noexcept;
};

class CanvasTransformPipeline {
public:
    static Transform2D buildTransformFromParams(const nlohmann::json& params);
    static double readOpacityFromParams(const nlohmann::json& params);
    static std::string readBlendModeFromParams(const nlohmann::json& params);

    static Transform2D resolveTransformAtTime(
        const Transform2D& baseTransform,
        const nlohmann::json& animationsJson,
        double localTimeSeconds
    );

    static Matrix3x3 getTransformMatrix(const Transform2D& transform) noexcept;
};

} // namespace catchim::render
