#include "image_render.h"

#include "image_render_base/hittable_list.h"

namespace image_render {

namespace {

using color = vec3;

bool hit_sphere(const point3& center, float radius, const ray& r) {
    vec3 oc = center - r.origin();
    auto a = dot(r.direction(), r.direction());
    auto b = -2.0f * dot(r.direction(), oc);
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    return (discriminant >= 0);
}

color image3_ray_color(const ray& r, const hittable&) {
    if (hit_sphere(point3(0.0f, 0.0f, -1.0f), 0.5f, r))
        return color(1.0f, 0.0f, 0.0f);

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

render_scene create_image3_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    scene.world = std::make_shared<hittable_list>();
    scene.ray_color = image3_ray_color;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image3", create_image3_scene}); }
} auto_reg;
} // namespace

} // namespace image_render
