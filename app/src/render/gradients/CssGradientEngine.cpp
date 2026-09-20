#include "render/gradients/CssGradientEngine.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace catchim::render {

namespace {

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return str;
}

std::vector<std::string> splitWithParentheses(const std::string& s) {
    std::vector<std::string> result;
    std::string current;
    int depth = 0;

    for (char c : s) {
        if (c == '(') {
            depth++;
        } else if (c == ')') {
            if (depth > 0) depth--;
        }

        if (c == ',' && depth == 0) {
            std::string t = trim(current);
            if (!t.empty()) result.push_back(t);
            current.clear();
        } else {
            current += c;
        }
    }

    std::string t = trim(current);
    if (!t.empty()) result.push_back(t);
    return result;
}

double hslToRgbHelper(double p, double q, double t) {
    if (t < 0.0) t += 1.0;
    if (t > 1.0) t -= 1.0;
    if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
    if (t < 1.0 / 2.0) return q;
    if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
    return p;
}

} // namespace

std::string CssRgba::toRgbaString() const {
    std::ostringstream oss;
    oss << "rgba(" << static_cast<int>(r) << ", "
        << static_cast<int>(g) << ", "
        << static_cast<int>(b) << ", "
        << a << ")";
    return oss.str();
}

std::string CssRgba::toHexString() const {
    std::ostringstream oss;
    oss << "#"
        << std::hex << std::setfill('0')
        << std::setw(2) << static_cast<int>(r)
        << std::setw(2) << static_cast<int>(g)
        << std::setw(2) << static_cast<int>(b);
    return oss.str();
}

CssRgba CssGradientEngine::parseColor(const std::string& str) {
    std::string s = toLower(trim(str));
    if (s.empty()) return {255, 255, 255, 1.0};

    if (s == "transparent") {
        return {0, 0, 0, 0.0};
    }
    if (s == "white") return {255, 255, 255, 1.0};
    if (s == "black") return {0, 0, 0, 1.0};
    if (s == "red") return {255, 0, 0, 1.0};
    if (s == "green") return {0, 128, 0, 1.0};
    if (s == "blue") return {0, 0, 255, 1.0};
    if (s == "yellow") return {255, 255, 0, 1.0};
    if (s == "cyan") return {0, 255, 255, 1.0};
    if (s == "magenta") return {255, 0, 255, 1.0};
    if (s == "gray" || s == "grey") return {128, 128, 128, 1.0};

    // Hex color #rgb, #rgba, #rrggbb, #rrggbbaa
    if (s[0] == '#') {
        std::string h = s.substr(1);
        if (h.size() == 3) {
            uint8_t r = static_cast<uint8_t>(std::stoi(std::string(2, h[0]), nullptr, 16));
            uint8_t g = static_cast<uint8_t>(std::stoi(std::string(2, h[1]), nullptr, 16));
            uint8_t b = static_cast<uint8_t>(std::stoi(std::string(2, h[2]), nullptr, 16));
            return {r, g, b, 1.0};
        }
        if (h.size() == 4) {
            uint8_t r = static_cast<uint8_t>(std::stoi(std::string(2, h[0]), nullptr, 16));
            uint8_t g = static_cast<uint8_t>(std::stoi(std::string(2, h[1]), nullptr, 16));
            uint8_t b = static_cast<uint8_t>(std::stoi(std::string(2, h[2]), nullptr, 16));
            double a = std::stoi(std::string(2, h[3]), nullptr, 16) / 255.0;
            return {r, g, b, a};
        }
        if (h.size() == 6) {
            uint8_t r = static_cast<uint8_t>(std::stoi(h.substr(0, 2), nullptr, 16));
            uint8_t g = static_cast<uint8_t>(std::stoi(h.substr(2, 2), nullptr, 16));
            uint8_t b = static_cast<uint8_t>(std::stoi(h.substr(4, 2), nullptr, 16));
            return {r, g, b, 1.0};
        }
        if (h.size() == 8) {
            uint8_t r = static_cast<uint8_t>(std::stoi(h.substr(0, 2), nullptr, 16));
            uint8_t g = static_cast<uint8_t>(std::stoi(h.substr(2, 2), nullptr, 16));
            uint8_t b = static_cast<uint8_t>(std::stoi(h.substr(4, 2), nullptr, 16));
            double a = std::stoi(h.substr(6, 2), nullptr, 16) / 255.0;
            return {r, g, b, a};
        }
    }

    // rgb(...) or rgba(...)
    if (s.rfind("rgb", 0) == 0) {
        auto openParen = s.find('(');
        auto closeParen = s.rfind(')');
        if (openParen != std::string::npos && closeParen != std::string::npos) {
            std::string inside = s.substr(openParen + 1, closeParen - openParen - 1);
            auto parts = splitWithParentheses(inside);
            if (parts.size() >= 3) {
                try {
                    int r = std::clamp(std::stoi(parts[0]), 0, 255);
                    int g = std::clamp(std::stoi(parts[1]), 0, 255);
                    int b = std::clamp(std::stoi(parts[2]), 0, 255);
                    double a = 1.0;
                    if (parts.size() >= 4) {
                        a = std::clamp(std::stod(parts[3]), 0.0, 1.0);
                    }
                    return {static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), a};
                } catch (...) {}
            }
        }
    }

    // hsl(...) or hsla(...)
    if (s.rfind("hsl", 0) == 0) {
        auto openParen = s.find('(');
        auto closeParen = s.rfind(')');
        if (openParen != std::string::npos && closeParen != std::string::npos) {
            std::string inside = s.substr(openParen + 1, closeParen - openParen - 1);
            auto parts = splitWithParentheses(inside);
            if (parts.size() >= 3) {
                try {
                    double h = std::stod(parts[0]);
                    while (h < 0.0) h += 360.0;
                    h = std::fmod(h, 360.0) / 360.0;

                    double sVal = std::stod(parts[1]);
                    if (parts[1].find('%') != std::string::npos) sVal /= 100.0;
                    sVal = std::clamp(sVal, 0.0, 1.0);

                    double lVal = std::stod(parts[2]);
                    if (parts[2].find('%') != std::string::npos) lVal /= 100.0;
                    lVal = std::clamp(lVal, 0.0, 1.0);

                    double a = 1.0;
                    if (parts.size() >= 4) {
                        a = std::clamp(std::stod(parts[3]), 0.0, 1.0);
                    }

                    double q = lVal < 0.5 ? lVal * (1.0 + sVal) : lVal + sVal - lVal * sVal;
                    double p = 2.0 * lVal - q;

                    double rNorm = hslToRgbHelper(p, q, h + 1.0 / 3.0);
                    double gNorm = hslToRgbHelper(p, q, h);
                    double bNorm = hslToRgbHelper(p, q, h - 1.0 / 3.0);

                    return {
                        static_cast<uint8_t>(std::clamp(rNorm * 255.0, 0.0, 255.0)),
                        static_cast<uint8_t>(std::clamp(gNorm * 255.0, 0.0, 255.0)),
                        static_cast<uint8_t>(std::clamp(bNorm * 255.0, 0.0, 255.0)),
                        a
                    };
                } catch (...) {}
            }
        }
    }

    return {255, 255, 255, 1.0};
}

std::vector<std::string> CssGradientEngine::splitCssLayers(const std::string& css) {
    return splitWithParentheses(css);
}

std::vector<BackgroundLayer> CssGradientEngine::parseBackgroundLayers(const std::string& css) {
    std::vector<BackgroundLayer> layers;
    auto segments = splitCssLayers(css);

    for (const auto& seg : segments) {
        if (seg.empty()) continue;
        auto gradOpt = parseGradient(seg);
        if (gradOpt.has_value()) {
            BackgroundLayer layer;
            layer.type = BackgroundLayerType::Gradient;
            layer.gradient = *gradOpt;
            layers.push_back(layer);
        } else {
            BackgroundLayer layer;
            layer.type = BackgroundLayerType::Color;
            layer.colorValue = seg;
            layers.push_back(layer);
        }
    }

    return layers;
}

std::optional<ParsedGradient> CssGradientEngine::parseGradient(const std::string& css) {
    std::string s = trim(css);
    if (s.empty()) return std::nullopt;

    // Strip webkit/moz/o prefixes
    std::string lower = toLower(s);
    if (lower.rfind("-webkit-", 0) == 0) lower = lower.substr(8);
    else if (lower.rfind("-moz-", 0) == 0) lower = lower.substr(5);
    else if (lower.rfind("-o-", 0) == 0) lower = lower.substr(3);

    ParsedGradient grad;
    if (lower.rfind("linear-gradient", 0) == 0) {
        grad.type = CssGradientType::Linear;
    } else if (lower.rfind("repeating-linear-gradient", 0) == 0) {
        grad.type = CssGradientType::RepeatingLinear;
    } else if (lower.rfind("radial-gradient", 0) == 0) {
        grad.type = CssGradientType::Radial;
    } else if (lower.rfind("repeating-radial-gradient", 0) == 0) {
        grad.type = CssGradientType::RepeatingRadial;
    } else {
        return std::nullopt;
    }

    auto openParen = lower.find('(');
    auto closeParen = lower.rfind(')');
    if (openParen == std::string::npos || closeParen == std::string::npos || closeParen <= openParen) {
        return std::nullopt;
    }

    std::string inside = s.substr(openParen + 1, closeParen - openParen - 1);
    auto parts = splitWithParentheses(inside);
    if (parts.empty()) return std::nullopt;

    size_t firstStopIdx = 0;
    std::string firstPart = toLower(trim(parts[0]));

    // Check if firstPart is orientation
    bool hasOrientation = false;
    if (grad.type == CssGradientType::Linear || grad.type == CssGradientType::RepeatingLinear) {
        if (firstPart.rfind("to ", 0) == 0) {
            hasOrientation = true;
            double dx = 0.0, dy = 0.0;
            if (firstPart.find("left") != std::string::npos) dx = -1.0;
            if (firstPart.find("right") != std::string::npos) dx = 1.0;
            if (firstPart.find("top") != std::string::npos) dy = -1.0;
            if (firstPart.find("bottom") != std::string::npos) dy = 1.0;

            if (dx == 0.0 && dy == 0.0) {
                grad.angleDegrees = 180.0;
            } else {
                double angle = std::atan2(dx, -dy) * 180.0 / M_PI;
                while (angle < 0.0) angle += 360.0;
                grad.angleDegrees = std::fmod(angle, 360.0);
            }
        } else if (firstPart.find("deg") != std::string::npos) {
            hasOrientation = true;
            try {
                double deg = std::stod(firstPart);
                while (deg < 0.0) deg += 360.0;
                grad.angleDegrees = std::fmod(deg, 360.0);
            } catch (...) {}
        } else if (firstPart.find("rad") != std::string::npos) {
            hasOrientation = true;
            try {
                double rad = std::stod(firstPart);
                double deg = rad * 180.0 / M_PI;
                while (deg < 0.0) deg += 360.0;
                grad.angleDegrees = std::fmod(deg, 360.0);
            } catch (...) {}
        } else if (firstPart.find("turn") != std::string::npos) {
            hasOrientation = true;
            try {
                double turn = std::stod(firstPart);
                double deg = turn * 360.0;
                while (deg < 0.0) deg += 360.0;
                grad.angleDegrees = std::fmod(deg, 360.0);
            } catch (...) {}
        }
    } else {
        // Radial gradient orientation
        if (firstPart.find("circle") != std::string::npos ||
            firstPart.find("ellipse") != std::string::npos ||
            firstPart.find("closest-") != std::string::npos ||
            firstPart.find("farthest-") != std::string::npos ||
            firstPart.find("at ") != std::string::npos) {
            hasOrientation = true;

            if (firstPart.find("circle") != std::string::npos) {
                grad.radialShape = RadialShape::Circle;
            } else {
                grad.radialShape = RadialShape::Ellipse;
            }

            if (firstPart.find("closest-side") != std::string::npos) {
                grad.radialExtent = RadialExtent::ClosestSide;
            } else if (firstPart.find("farthest-side") != std::string::npos) {
                grad.radialExtent = RadialExtent::FarthestSide;
            } else if (firstPart.find("closest-corner") != std::string::npos) {
                grad.radialExtent = RadialExtent::ClosestCorner;
            } else {
                grad.radialExtent = RadialExtent::FarthestCorner;
            }

            auto atPos = firstPart.find("at ");
            if (atPos != std::string::npos) {
                std::string atStr = firstPart.substr(atPos + 3);
                std::istringstream iss(atStr);
                std::string xPos, yPos;
                iss >> xPos >> yPos;

                if (xPos == "center") grad.centerXRatio = 0.5;
                else if (xPos == "left") grad.centerXRatio = 0.0;
                else if (xPos == "right") grad.centerXRatio = 1.0;
                else if (xPos.find('%') != std::string::npos) {
                    try { grad.centerXRatio = std::stod(xPos) / 100.0; } catch (...) {}
                }

                if (yPos == "center") grad.centerYRatio = 0.5;
                else if (yPos == "top") grad.centerYRatio = 0.0;
                else if (yPos == "bottom") grad.centerYRatio = 1.0;
                else if (yPos.find('%') != std::string::npos) {
                    try { grad.centerYRatio = std::stod(yPos) / 100.0; } catch (...) {}
                } else if (yPos.empty()) {
                    grad.centerYRatio = 0.5;
                }
            }
        }
    }

    if (hasOrientation) {
        firstStopIdx = 1;
    }

    // Parse color stops
    for (size_t i = firstStopIdx; i < parts.size(); ++i) {
        std::string part = trim(parts[i]);
        if (part.empty()) continue;

        // Separate color and offset: find last space
        std::string colorStr = part;
        std::optional<double> offset;

        // Check if there is a trailing percentage or position
        auto lastSpace = part.find_last_of(" \t");
        if (lastSpace != std::string::npos) {
            std::string potentialOffset = part.substr(lastSpace + 1);
            if (potentialOffset.find('%') != std::string::npos) {
                try {
                    offset = std::stod(potentialOffset) / 100.0;
                    colorStr = trim(part.substr(0, lastSpace));
                } catch (...) {}
            } else if (potentialOffset.find("px") != std::string::npos) {
                // Approximate px offset as fraction (will be normalized later if needed)
                try {
                    offset = std::stod(potentialOffset) / 1000.0;
                    colorStr = trim(part.substr(0, lastSpace));
                } catch (...) {}
            }
        }

        CssColorStop stop;
        stop.rawColor = colorStr;
        stop.color = parseColor(colorStr);
        stop.offset = offset;
        grad.colorStops.push_back(stop);
    }

    grad.colorStops = normalizeColorStops(grad.colorStops);
    grad.colorStops = fixTransparentStops(grad.colorStops);

    return grad;
}

LinearPoints CssGradientEngine::resolveLinearPoints(
    double width,
    double height,
    double angleDegrees
) {
    double rad = angleDegrees * M_PI / 180.0;
    double dx = std::sin(rad);
    double dy = -std::cos(rad);
    double cx = width / 2.0;
    double cy = height / 2.0;
    double halfLen = (std::abs(width * dx) + std::abs(height * dy)) / 2.0;

    LinearPoints pts;
    pts.x0 = cx - dx * halfLen;
    pts.y0 = cy - dy * halfLen;
    pts.x1 = cx + dx * halfLen;
    pts.y1 = cy + dy * halfLen;
    pts.length = std::hypot(pts.x1 - pts.x0, pts.y1 - pts.y0);
    return pts;
}

RadialDimensions CssGradientEngine::resolveRadialDimensions(
    double width,
    double height,
    RadialShape shape,
    RadialExtent extent,
    double centerXRatio,
    double centerYRatio
) {
    double cx = width * centerXRatio;
    double cy = height * centerYRatio;
    double left = cx;
    double right = width - cx;
    double top = cy;
    double bottom = height - cy;

    RadialDimensions dims;
    dims.cx = cx;
    dims.cy = cy;

    if (shape == RadialShape::Circle) {
        double dists[4] = {
            std::hypot(left, top),
            std::hypot(right, top),
            std::hypot(left, bottom),
            std::hypot(right, bottom)
        };
        if (extent == RadialExtent::ClosestSide) {
            dims.rx = dims.ry = std::min({left, right, top, bottom});
        } else if (extent == RadialExtent::FarthestSide) {
            dims.rx = dims.ry = std::max({left, right, top, bottom});
        } else if (extent == RadialExtent::ClosestCorner) {
            dims.rx = dims.ry = std::min({dists[0], dists[1], dists[2], dists[3]});
        } else {
            dims.rx = dims.ry = std::max({dists[0], dists[1], dists[2], dists[3]});
        }
        return dims;
    }

    // Ellipse
    if (extent == RadialExtent::ClosestSide) {
        dims.rx = std::min(left, right);
        dims.ry = std::min(top, bottom);
    } else if (extent == RadialExtent::FarthestSide) {
        dims.rx = std::max(left, right);
        dims.ry = std::max(top, bottom);
    } else {
        // Corner distance calculation
        double cornerX = std::max(left, right);
        double cornerY = std::max(top, bottom);
        if (extent == RadialExtent::ClosestCorner) {
            cornerX = std::min(left, right);
            cornerY = std::min(top, bottom);
        }
        dims.rx = cornerX * std::sqrt(2.0);
        dims.ry = cornerY * std::sqrt(2.0);
    }

    return dims;
}

std::vector<CssColorStop> CssGradientEngine::normalizeColorStops(
    const std::vector<CssColorStop>& stops
) {
    if (stops.empty()) return stops;
    std::vector<CssColorStop> result = stops;

    if (!result.front().offset.has_value()) {
        result.front().offset = 0.0;
    }
    if (!result.back().offset.has_value()) {
        result.back().offset = 1.0;
    }

    // Distribute missing offsets between known offsets
    size_t i = 0;
    while (i < result.size()) {
        if (result[i].offset.has_value()) {
            size_t nextKnown = i + 1;
            while (nextKnown < result.size() && !result[nextKnown].offset.has_value()) {
                nextKnown++;
            }
            if (nextKnown < result.size()) {
                double startOffset = *result[i].offset;
                double endOffset = *result[nextKnown].offset;
                size_t stepCount = nextKnown - i;
                for (size_t k = i + 1; k < nextKnown; ++k) {
                    double t = static_cast<double>(k - i) / static_cast<double>(stepCount);
                    result[k].offset = startOffset + t * (endOffset - startOffset);
                }
            }
            i = nextKnown;
        } else {
            i++;
        }
    }

    return result;
}

std::vector<CssColorStop> CssGradientEngine::fixTransparentStops(
    const std::vector<CssColorStop>& stops
) {
    if (stops.empty()) return stops;
    std::vector<CssColorStop> result = stops;

    for (size_t i = 0; i < result.size(); ++i) {
        if (!result[i].color.isTransparent()) {
            continue;
        }

        const CssRgba* donor = nullptr;
        // Search backwards
        for (int j = static_cast<int>(i) - 1; j >= 0; --j) {
            if (!result[j].color.isTransparent()) {
                donor = &result[j].color;
                break;
            }
        }
        // Search forwards if not found
        if (!donor) {
            for (size_t j = i + 1; j < result.size(); ++j) {
                if (!result[j].color.isTransparent()) {
                    donor = &result[j].color;
                    break;
                }
            }
        }

        if (donor) {
            result[i].color.r = donor->r;
            result[i].color.g = donor->g;
            result[i].color.b = donor->b;
            result[i].color.a = 0.0;
        }
    }

    return result;
}

const std::vector<std::string>& CssGradientEngine::getPatternCraftGradients() {
    static const std::vector<std::string> gradients = {
        "radial-gradient(circle at 30% 70%, rgba(173, 216, 230, 0.35), transparent 60%), radial-gradient(circle at 70% 30%, rgba(255, 182, 193, 0.4), transparent 60%), white",
        "radial-gradient(circle at 20% 80%, rgba(255, 182, 153, 0.3) 0%, transparent 50%), radial-gradient(circle at 80% 20%, rgba(255, 244, 214, 0.5) 0%, transparent 50%), #fff8f0",
        "radial-gradient(circle at center, #8FFFB0, transparent), white",
        "radial-gradient(circle at top right, rgba(173, 109, 244, 0.5), transparent 70%), white",
        "radial-gradient(circle at top right, rgba(56, 193, 182, 0.5), transparent 70%), white",
        "radial-gradient(circle at top right, rgba(255, 140, 60, 0.5), transparent 70%), white",
        "linear-gradient(120deg, #d5c5ff 0%, #a7f3d0 50%, #f0f0f0 100%)",
        "linear-gradient(135deg, #F8BBD9 0%, #FDD5B4 25%, #FFF2CC 50%, #E1F5FE 75%, #BBDEFB 100%)",
        "linear-gradient(45deg, #FFB3D9 0%, #FFD1DC 20%, #FFF0F5 40%, #E6F3FF 60%, #D1E7FF 80%, #C7E9F1 100%)",
        "linear-gradient(270deg, #FFECB3 0%, #FFE0B2 20%, #FFCDD2 40%, #F8BBD9 60%, #E1BEE7 80%, #D1C4E9 100%)"
    };
    return gradients;
}

const std::vector<std::string>& CssGradientEngine::getSolidColorPalette() {
    static const std::vector<std::string> colors = {
        "#ffffff", "#000000", "#ffe2e2", "#ffc9c9", "#ffa2a2", "#ff6467", "#fb2c36",
        "#e7000b", "#c10007", "#9f0712", "#82181a", "#460809", "#fff7ed", "#ffedd4",
        "#ffd6a7", "#ffb86a", "#ff8904", "#ff6900", "#f54900", "#ca3500", "#9f2d00",
        "#7e2a0c", "#441306", "#fffbeb", "#fef3c6", "#fee685", "#ffd230", "#ffb900"
    };
    return colors;
}

} // namespace catchim::render
