#include "render/canvas/CanvasTransformPipeline.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::render {

namespace {

double interpolateChannel(const nlohmann::json& channelJson, double localTime, double fallback) {
    if (!channelJson.is_object() || !channelJson.contains("keys") || !channelJson["keys"].is_array()) {
        return fallback;
    }

    const auto& keys = channelJson["keys"];
    if (keys.empty()) {
        return fallback;
    }

    // Keys are assumed to have "time" and "value"
    if (keys.size() == 1) {
        return keys[0].value("value", fallback);
    }

    double firstTime = keys.front().value("time", 0.0);
    if (localTime <= firstTime) {
        return keys.front().value("value", fallback);
    }

    double lastTime = keys.back().value("time", 0.0);
    if (localTime >= lastTime) {
        return keys.back().value("value", fallback);
    }

    // Binary search or linear scan for segment
    for (size_t i = 0; i + 1 < keys.size(); ++i) {
        double t0 = keys[i].value("time", 0.0);
        double t1 = keys[i + 1].value("time", 0.0);
        if (localTime >= t0 && localTime <= t1) {
            double v0 = keys[i].value("value", fallback);
            double v1 = keys[i + 1].value("value", fallback);
            if (std::abs(t1 - t0) < 1e-9) {
                return v0;
            }
            double ratio = (localTime - t0) / (t1 - t0);
            return v0 + ratio * (v1 - v0);
        }
    }

    return fallback;
}

} // namespace

Matrix3x3 Matrix3x3::identity() noexcept {
    return Matrix3x3{};
}

Matrix3x3 Matrix3x3::translation(double tx, double ty) noexcept {
    Matrix3x3 res = identity();
    res.m[0][2] = tx;
    res.m[1][2] = ty;
    return res;
}

Matrix3x3 Matrix3x3::rotation(double radians) noexcept {
    Matrix3x3 res = identity();
    double c = std::cos(radians);
    double s = std::sin(radians);
    res.m[0][0] = c;
    res.m[0][1] = -s;
    res.m[1][0] = s;
    res.m[1][1] = c;
    return res;
}

Matrix3x3 Matrix3x3::scale(double sx, double sy) noexcept {
    Matrix3x3 res = identity();
    res.m[0][0] = sx;
    res.m[1][1] = sy;
    return res;
}

Matrix3x3 Matrix3x3::multiply(const Matrix3x3& a, const Matrix3x3& b) noexcept {
    Matrix3x3 res;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            res.m[r][c] = a.m[r][0] * b.m[0][c] +
                          a.m[r][1] * b.m[1][c] +
                          a.m[r][2] * b.m[2][c];
        }
    }
    return res;
}

Matrix3x3 Matrix3x3::compose(double tx, double ty, double rotDegrees, double sx, double sy) noexcept {
    double rad = rotDegrees * M_PI / 180.0;
    double c = std::cos(rad);
    double s = std::sin(rad);

    Matrix3x3 res;
    res.m[0][0] = sx * c;
    res.m[0][1] = -sy * s;
    res.m[0][2] = tx;

    res.m[1][0] = sx * s;
    res.m[1][1] = sy * c;
    res.m[1][2] = ty;

    res.m[2][0] = 0.0;
    res.m[2][1] = 0.0;
    res.m[2][2] = 1.0;

    return res;
}

double Matrix3x3::determinant() const noexcept {
    return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

std::optional<Matrix3x3> Matrix3x3::inverse() const noexcept {
    double det = determinant();
    if (std::abs(det) < 1e-9) {
        return std::nullopt;
    }

    double invDet = 1.0 / det;
    Matrix3x3 inv;
    inv.m[0][0] = m[1][1] * invDet;
    inv.m[0][1] = -m[0][1] * invDet;
    inv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;

    inv.m[1][0] = -m[1][0] * invDet;
    inv.m[1][1] = m[0][0] * invDet;
    inv.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet;

    inv.m[2][0] = 0.0;
    inv.m[2][1] = 0.0;
    inv.m[2][2] = 1.0;

    return inv;
}

Point2D Matrix3x3::transformPoint(const Point2D& p) const noexcept {
    Point2D res;
    res.x = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
    res.y = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
    return res;
}

std::optional<Point2D> Matrix3x3::inverseTransformPoint(const Point2D& p) const noexcept {
    auto invOpt = inverse();
    if (!invOpt.has_value()) {
        return std::nullopt;
    }
    return invOpt->transformPoint(p);
}

Transform2D CanvasTransformPipeline::buildTransformFromParams(const nlohmann::json& params) {
    Transform2D t;
    if (!params.is_object()) {
        return t;
    }

    t.scaleX = params.value("transform.scaleX", 1.0);
    t.scaleY = params.value("transform.scaleY", 1.0);
    t.positionX = params.value("transform.positionX", 0.0);
    t.positionY = params.value("transform.positionY", 0.0);
    t.rotate = params.value("transform.rotate", 0.0);

    return t;
}

double CanvasTransformPipeline::readOpacityFromParams(const nlohmann::json& params) {
    if (!params.is_object()) return 1.0;
    return params.value("opacity", 1.0);
}

std::string CanvasTransformPipeline::readBlendModeFromParams(const nlohmann::json& params) {
    if (!params.is_object()) return "normal";
    return params.value("blendMode", "normal");
}

Transform2D CanvasTransformPipeline::resolveTransformAtTime(
    const Transform2D& baseTransform,
    const nlohmann::json& animationsJson,
    double localTimeSeconds
) {
    Transform2D res = baseTransform;
    double safeTime = std::max(0.0, localTimeSeconds);

    if (animationsJson.is_object()) {
        if (animationsJson.contains("transform.positionX")) {
            res.positionX = interpolateChannel(animationsJson["transform.positionX"], safeTime, baseTransform.positionX);
        }
        if (animationsJson.contains("transform.positionY")) {
            res.positionY = interpolateChannel(animationsJson["transform.positionY"], safeTime, baseTransform.positionY);
        }
        if (animationsJson.contains("transform.scaleX")) {
            res.scaleX = interpolateChannel(animationsJson["transform.scaleX"], safeTime, baseTransform.scaleX);
        }
        if (animationsJson.contains("transform.scaleY")) {
            res.scaleY = interpolateChannel(animationsJson["transform.scaleY"], safeTime, baseTransform.scaleY);
        }
        if (animationsJson.contains("transform.rotate")) {
            res.rotate = interpolateChannel(animationsJson["transform.rotate"], safeTime, baseTransform.rotate);
        }
    }

    return res;
}

Matrix3x3 CanvasTransformPipeline::getTransformMatrix(const Transform2D& transform) noexcept {
    return Matrix3x3::compose(
        transform.positionX,
        transform.positionY,
        transform.rotate,
        transform.scaleX,
        transform.scaleY
    );
}

} // namespace catchim::render
