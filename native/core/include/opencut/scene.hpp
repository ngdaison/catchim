#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "opencut/compositor.hpp"

namespace opencut::scene {

enum class NodeType : uint32_t {
    Video = 0,
    Image = 1,
    Text = 2,
    Color = 3,
    Sticker = 4,
    Blur = 5,
    EffectLayer = 6
};

struct Rect2D {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    [[nodiscard]] bool intersects(const Rect2D& other) const noexcept {
        return !(x + width < other.x || other.x + other.width < x ||
                 y + height < other.y || other.y + other.height < y);
    }
};

struct RenderItem {
    std::string id;
    NodeType type = NodeType::Video;
    int32_t z_index = 0;
    bool visible = true;
    float opacity = 1.0f;
    uint32_t blend_mode = 0; // BlendMode::Normal
    QuadTransform local_transform{};
    Matrix3x3 world_matrix = Matrix3x3::identity();
    Rect2D world_bounds{};
    std::string asset_id;
    ColorRGBA color{0.0f, 0.0f, 0.0f, 1.0f};
};

class SceneGraph {
public:
    SceneGraph() = default;

    void add_item(RenderItem item);
    void clear();

    [[nodiscard]] size_t size() const noexcept { return items_.size(); }
    [[nodiscard]] bool empty() const noexcept { return items_.empty(); }

    // Generates a sorted, view-frustum culled display list in back-to-front Z-order
    [[nodiscard]] std::vector<RenderItem> build_display_list(
        double viewport_width,
        double viewport_height,
        bool enable_culling = true
    ) const;

private:
    std::vector<RenderItem> items_;
};

} // namespace opencut::scene
