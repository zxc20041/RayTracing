#ifndef IMAGE_RENDER_H
#define IMAGE_RENDER_H

#include "image_render_base/camera.h"
#include "image_render_base/hittable.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace image_render {

struct render_scene {
    camera cam;
    std::shared_ptr<hittable> world;
    camera::ray_color_fn ray_color;
    camera::ray_color_depth_fn ray_color_with_depth = nullptr;
    
    // Optional: custom fill_scanline implementation
    using fill_scanline_fn = void (*)(int j, const hittable&, std::vector<uint8_t>&, const render_scene&);
    fill_scanline_fn fill_scanline_override = nullptr;
};

struct render_entry {
    const char* name;
    render_scene (*create_scene)();
};

void register_renderer(const render_entry& entry);
const std::vector<render_entry>& get_renderers();

render_scene create_image2_scene();
render_scene create_image3_scene();
render_scene create_image4_scene();
render_scene create_image5_scene();
render_scene create_image6_scene();
render_scene create_image6_2_scene();
render_scene create_image7_scene();
render_scene create_image8_scene();
render_scene create_image9_scene();
render_scene create_image10_scene();

} // namespace image_render

#endif
