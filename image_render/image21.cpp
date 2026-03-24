#include "image_render_base/rtweekend.h"
#include "image_render_base/hittable.h"
#include "image_render_base/hittable_list.h"
#include "image_render_base/sphere.h"
#include "image_render_base/material.h"
#include "image_render.h"

namespace image_render {

namespace {

using color = vec3;

color image21_ray_color(const ray& r, const hittable& world, int depth) {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0.0f, 0.0f, 0.0f);
    hit_record rec;
    if (world.hit(r, interval(0.001f, infinity), rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * image21_ray_color(scattered, world, depth-1);
        return color(0.0f, 0.0f, 0.0f);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

render_scene create_image21_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    scene.cam.samples_per_pixel = 50;
    scene.cam.max_depth         = 50;
    scene.cam.enable_gamma_correction = true;
    scene.cam.vfov = 20;
    scene.cam.lookfrom = point3(-2,2,1);
    scene.cam.lookat   = point3(0,0,-1);
    scene.cam.vup      = vec3(0,1,0);

    auto world = std::make_shared<hittable_list>();

    auto material_ground = make_shared<lambertian>(color(0.8f, 0.8f, 0.0f));
    auto material_center = make_shared<lambertian>(color(0.1f, 0.2f, 0.5f));
    auto material_left   = make_shared<dielectric>(1.50f);
    auto material_bubble = make_shared<dielectric>(1.00f / 1.50f);
    auto material_right  = make_shared<metal>(color(0.8f, 0.6f, 0.2f), 1.0f);

    world->add(make_shared<sphere>(point3( 0.0f, -100.5, -1.0f), 100.0f, material_ground));
    world->add(make_shared<sphere>(point3( 0.0f,    0.0f, -1.2f),   0.5f, material_center));
    world->add(make_shared<sphere>(point3(-1.0f,    0.0f, -1.0f),   0.5f, material_left));
    world->add(make_shared<sphere>(point3(-1.0f,    0.0f, -1.0f),   0.4f, material_bubble));
    world->add(make_shared<sphere>(point3( 1.0f,    0.0f, -1.0f),   0.5f, material_right));

    scene.world = world;
    scene.ray_color = nullptr;
    scene.ray_color_with_depth = image21_ray_color;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image21", create_image21_scene}); }
} auto_reg;
} // namespace

} // namespace image_render