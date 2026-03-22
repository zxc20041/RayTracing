#include "image_render_base/rtweekend.h"
#include "image_render_base/hittable.h"
#include "image_render_base/hittable_list.h"
#include "image_render_base/sphere.h"
#include "image_render.h"

#include <cstdint>
#include <vector>

namespace image_render {

namespace {

using color = vec3;

color image11_ray_color(const ray& r, const hittable& world, int depth, float reflectance) {
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0.0f, 0.0f, 0.0f);

    hit_record rec;
    if (world.hit(r, interval(0.001f, infinity), rec)) {
        vec3 direction = rec.normal + random_unit_vector();
        return reflectance * image11_ray_color(ray(rec.p, direction), world, depth - 1, reflectance);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

void fill_scanline_reflectance_bands(int j, const hittable& world, std::vector<uint8_t>& pixels, const render_scene& scene) {
    const int image_width = scene.cam.image_width;

    // Left to right: 10%, 30%, 50%, 70%, 90%
    static constexpr float reflectance_bands[5] = {0.1f, 0.3f, 0.5f, 0.7f, 0.9f};
    static const interval intensity(0.000f, 0.999f);

    for (int i = 0; i < image_width; i++) {
        int band = (i * 5) / image_width;
        if (band < 0) band = 0;
        if (band > 4) band = 4;

        const float reflectance = reflectance_bands[band];

        color pixel_color(0.0f, 0.0f, 0.0f);
        for (int sample = 0; sample < scene.cam.samples_per_pixel; sample++) {
            ray r = scene.cam.get_ray(i, j);
            pixel_color += image11_ray_color(r, world, scene.cam.max_depth, reflectance);
        }

        pixel_color *= (1.0f / static_cast<float>(scene.cam.samples_per_pixel));

        int idx = (j * image_width + i) * 3;
        pixels[idx + 0] = static_cast<uint8_t>(256.0f * intensity.clamp(pixel_color.x()));
        pixels[idx + 1] = static_cast<uint8_t>(256.0f * intensity.clamp(pixel_color.y()));
        pixels[idx + 2] = static_cast<uint8_t>(256.0f * intensity.clamp(pixel_color.z()));
    }
}

} // namespace

render_scene create_image11_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    scene.cam.samples_per_pixel = 25;
    scene.cam.max_depth         = 50;

    auto world = std::make_shared<hittable_list>();
    world->add(make_shared<sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f));
    world->add(make_shared<sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f));

    scene.world = world;
    scene.ray_color = nullptr;
    scene.ray_color_with_depth = nullptr;
    scene.fill_scanline_override = fill_scanline_reflectance_bands;
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image11", create_image11_scene}); }
} auto_reg;
} // namespace

} // namespace image_render