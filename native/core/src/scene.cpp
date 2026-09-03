#include "opencut/scene.hpp"
#include <algorithm>
#include <limits>

namespace opencut::scene {

void SceneGraph::add_item(RenderItem item) {
    items_.push_back(std::move(item));
}

void SceneGraph::clear() {
    items_.clear();
}

std::vector<RenderItem> SceneGraph::build_display_list(
    double viewport_width,
    double viewport_height,
    bool enable_culling
) const {
    std::vector<RenderItem> display_list;
    display_list.reserve(items_.size());

    const Rect2D viewport{0.0, 0.0, viewport_width, viewport_height};

    for (auto item : items_) {
        if (!item.visible || item.opacity <= 0.0f) {
            continue;
        }

        // Calculate world transform matrix
        item.world_matrix = Matrix3x3::from_transform(item.local_transform);

        // Calculate world bounding box from 4 quad corners
        const float hw = item.local_transform.width * 0.5f;
        const float hh = item.local_transform.height * 0.5f;

        const Point2D local_corners[4] = {
            {-hw, -hh},
            { hw, -hh},
            { hw,  hh},
            {-hw,  hh}
        };

        double min_x = std::numeric_limits<double>::infinity();
        double max_x = -std::numeric_limits<double>::infinity();
        double min_y = std::numeric_limits<double>::infinity();
        double max_y = -std::numeric_limits<double>::infinity();

        for (const auto& pt : local_corners) {
            const Point2D world_pt = item.world_matrix.transform_point(pt);
            const double px = static_cast<double>(world_pt.x);
            const double py = static_cast<double>(world_pt.y);
            min_x = std::min(min_x, px);
            max_x = std::max(max_x, px);
            min_y = std::min(min_y, py);
            max_y = std::max(max_y, py);
        }

        item.world_bounds = Rect2D{
            min_x,
            min_y,
            std::max(0.0, max_x - min_x),
            std::max(0.0, max_y - min_y)
        };

        if (enable_culling && viewport_width > 0 && viewport_height > 0) {
            if (!item.world_bounds.intersects(viewport)) {
                continue;
            }
        }

        display_list.push_back(std::move(item));
    }

    // Sort back-to-front by Z-index (painter's algorithm)
    std::stable_sort(display_list.begin(), display_list.end(),
        [](const RenderItem& a, const RenderItem& b) {
            return a.z_index < b.z_index;
        });

    return display_list;
}

} // namespace opencut::scene
