#include "image_render_base/rtweekend.h"
#include "image_render_base/hittable.h"
#include "image_render_base/hittable_list.h"
#include "image_render_base/sphere.h"
#include "image_render_base/material.h"
#include "image_render.h"

namespace image_render {

namespace {

using color = vec3;

color image23_ray_color(const ray& r, const hittable& world, int depth) {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0.0f, 0.0f, 0.0f);
    hit_record rec;
    if (world.hit(r, interval(0.001f, infinity), rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * image23_ray_color(scattered, world, depth-1);
        return color(0.0f, 0.0f, 0.0f);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

render_scene create_image23_scene() {
    render_scene scene;
    scene.cam.aspect_ratio      = 16.0f / 9.0f;
    scene.cam.image_width       = 1000;
    scene.cam.samples_per_pixel = 100;
    scene.cam.max_depth         = 50;

    scene.cam.enable_gamma_correction = true;

    scene.cam.vfov     = 20;
    scene.cam.lookfrom = point3(13,2,3);
    scene.cam.lookat   = point3(0,0,0);
    scene.cam.vup      = vec3(0,1,0);

    scene.cam.defocus_angle = 0.6f;
    scene.cam.focus_dist    = 10.0f;

    auto world = std::make_shared<hittable_list>();

    auto ground_material = make_shared<lambertian>(color(0.5f, 0.5f, 0.5f));
    world->add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_float();
            point3 center(a + 0.9f*random_float(), 0.2f, b + 0.9f*random_float());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    world->add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95f) {
                    // metal
                    auto albedo = color::random(0.5f, 1.0f);
                    auto fuzz = random_float(0.0f, 0.5f);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world->add(make_shared<sphere>(center, 0.2f, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5f);
                    world->add(make_shared<sphere>(center, 0.2f, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5f);
    world->add(make_shared<sphere>(point3(0, 1, 0), 1.0f, material1));

    auto material2 = make_shared<lambertian>(color(0.4f, 0.2f, 0.1f));
    world->add(make_shared<sphere>(point3(-4, 1, 0), 1.0f, material2));

    auto material3 = make_shared<metal>(color(0.7f, 0.6f, 0.5f), 0.0f);
    world->add(make_shared<sphere>(point3(4, 1, 0), 1.0f, material3));

    scene.world = world;
    scene.ray_color = nullptr;
    scene.ray_color_with_depth = image23_ray_color;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image23", create_image23_scene}); }
} auto_reg;
} // namespace

} // namespace image_render