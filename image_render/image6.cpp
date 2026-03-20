#include "image_render_base/rtweekend.h"
#include "image_render_base/hittable.h"
#include "image_render_base/hittable_list.h"
#include "image_render_base/sphere.h"
#include "image_render.h"

namespace image_render {

namespace {

using color = vec3;

color image6_ray_color(const ray& r, const hittable& world) {
    hit_record rec;
    if (world.hit(r, interval(0.0f, infinity), rec)) {
        return 0.5f * (rec.normal + color(1.0f, 1.0f, 1.0f));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

render_scene create_image6_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    scene.cam.samples_per_pixel = 100;

    auto world = std::make_shared<hittable_list>();
    world->add(make_shared<sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f));
    world->add(make_shared<sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f));

    scene.world = world;
    scene.ray_color = image6_ray_color;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image6", create_image6_scene}); }
} auto_reg;
} // namespace

} // namespace image_render