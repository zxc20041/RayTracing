#include "image_render.h"

#include "image_render_base/hittable_list.h"

namespace image_render {

namespace {

using color = vec3;

float hit_sphere(const point3& center, float radius, const ray& r) {
    vec3 oc = center - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), oc);
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = h * h - a * c;

    if (discriminant < 0) {
        return -1.0f;
    }

    return (h - std::sqrt(discriminant)) / a;
}

color image4_ray_color(const ray& r, const hittable&) {
    auto t = hit_sphere(point3(0.0f, 0.0f, -1.0f), 0.5f, r);
    if (t > 0.0f) {
        vec3 n = unit_vector(r.at(t) - vec3(0.0f, 0.0f, -1.0f));
        return 0.5f * color(n.x() + 1.0f, n.y() + 1.0f, n.z() + 1.0f);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

render_scene create_image4_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    scene.world = std::make_shared<hittable_list>();
    scene.ray_color = image4_ray_color;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image4", create_image4_scene}); }
} auto_reg;
} // namespace

} // namespace image_render
