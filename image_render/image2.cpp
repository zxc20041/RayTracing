#include "image_render.h"

#include "image_render_base/hittable_list.h"

namespace image_render {

namespace {

using color = vec3;

color image2_ray_color(const ray& r, const hittable&) {
    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

render_scene create_image2_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    scene.world = std::make_shared<hittable_list>();
    scene.ray_color = image2_ray_color;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image2", create_image2_scene}); }
} auto_reg;
} // namespace

} // namespace image_render
