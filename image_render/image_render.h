#ifndef IMAGE_RENDER_H
#define IMAGE_RENDER_H

#include "image_render_base/camera.h"
#include "image_render_base/hittable.h"

#include <memory>
#include <vector>

namespace image_render {

struct render_scene {
    camera cam;
    std::shared_ptr<hittable> world;
    camera::ray_color_fn ray_color;
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

} // namespace image_render

#endif
