#include "opencut/scene.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace opencut::scene;

static void test_scene_z_sorting() {
    SceneGraph graph;

    RenderItem item1;
    item1.id = "background";
    item1.z_index = 0;
    item1.local_transform = {960.0f, 540.0f, 1920.0f, 1080.0f, 0.0f, false, false};

    RenderItem item2;
    item2.id = "foreground";
    item2.z_index = 10;
    item2.local_transform = {960.0f, 540.0f, 200.0f, 200.0f, 0.0f, false, false};

    RenderItem item3;
    item3.id = "text";
    item3.z_index = 5;
    item3.local_transform = {960.0f, 540.0f, 400.0f, 100.0f, 0.0f, false, false};

    // Insert out of order
    graph.add_item(item2);
    graph.add_item(item1);
    graph.add_item(item3);

    auto display_list = graph.build_display_list(1920, 1080);
    assert(display_list.size() == 3);
    assert(display_list[0].id == "background");
    assert(display_list[1].id == "text");
    assert(display_list[2].id == "foreground");

    std::cout << "[PASS] test_scene_z_sorting\n";
}

static void test_scene_culling() {
    SceneGraph graph;

    // Item inside viewport [0, 1920] x [0, 1080]
    RenderItem inside;
    inside.id = "inside";
    inside.local_transform = {100.0f, 100.0f, 50.0f, 50.0f, 0.0f, false, false};

    // Item far outside viewport
    RenderItem outside;
    outside.id = "outside";
    outside.local_transform = {5000.0f, 5000.0f, 50.0f, 50.0f, 0.0f, false, false};

    graph.add_item(inside);
    graph.add_item(outside);

    auto display_list = graph.build_display_list(1920, 1080, true);
    assert(display_list.size() == 1);
    assert(display_list[0].id == "inside");

    std::cout << "[PASS] test_scene_culling\n";
}

int main() {
    test_scene_z_sorting();
    test_scene_culling();
    std::cout << "All scene tests passed successfully!\n";
    return 0;
}
