#include "image_render_base/rtweekend.h"
#include "image_render_base/hittable.h"
#include "image_render_base/hittable_list.h"
#include "image_render_base/sphere.h"
#include "image_render_base/interval.h"
#include "image_render.h"

#include <cstdint>
#include <vector>

namespace image_render {

namespace {

using color = vec3;

static void write_u8_pixel(std::vector<uint8_t>& buffer, int width, int x, int y, const color& c) {
    static const interval intensity(0.000f, 0.999f);
    int idx = (y * width + x) * 3;
    buffer[idx + 0] = static_cast<uint8_t>(256.0f * intensity.clamp(c.x()));
    buffer[idx + 1] = static_cast<uint8_t>(256.0f * intensity.clamp(c.y()));
    buffer[idx + 2] = static_cast<uint8_t>(256.0f * intensity.clamp(c.z()));
}

static color sample_color_at(
    const render_scene& scene,
    const hittable& world,
    float source_x,
    float source_y,
    int samples,
    bool jitter) {
    color pixel_color(0.0f, 0.0f, 0.0f);

    for (int s = 0; s < samples; s++) {
        ray r = scene.cam.get_ray(source_x, source_y, jitter);
        pixel_color += scene.cam.shade(r, world);
    }

    pixel_color *= (1.0f / static_cast<float>(samples));
    return pixel_color;
}

color image6_2_ray_color(const ray& r, const hittable& world) {
    hit_record rec;
    if (world.hit(r, interval(0.0f, infinity), rec)) {
        return 0.5f * (rec.normal + color(1.0f, 1.0f, 1.0f));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5f * (unit_direction.y() + 1.0f);
    return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

// Custom fill_scanline for split-screen comparison
// Left half: no anti-aliasing (1 sample per pixel)
// Right half: multi-sample anti-aliasing (100 samples per pixel)
void fill_scanline_comparison(int j, const hittable& world, std::vector<uint8_t>& pixels, const render_scene& scene) {
    const int image_width = scene.cam.image_width;
    const int image_height = scene.cam.image_height();
    const int half_width = image_width / 2;

    struct comparison_cache {
        int w = 0;
        int h = 0;
        int roi_w = 80;
        int roi_h = 80;
        int scale = 1;
        int pane_w = 0;
        int pane_h = 0;
        int left_x0 = 0;
        int right_x0 = 0;
        int y0 = 0;
        std::vector<uint8_t> left_roi;
        std::vector<uint8_t> right_roi;
    };

    static comparison_cache cache;

    if (cache.w != image_width || cache.h != image_height || j == 0) {
        cache.w = image_width;
        cache.h = image_height;

        cache.scale = half_width / cache.roi_w;
        int scale_by_h = image_height / cache.roi_h;
        if (scale_by_h < cache.scale) cache.scale = scale_by_h;
        if (cache.scale < 1) cache.scale = 1;

        cache.pane_w = cache.roi_w * cache.scale;
        cache.pane_h = cache.roi_h * cache.scale;
        cache.left_x0 = (half_width - cache.pane_w) / 2;
        cache.right_x0 = half_width + (half_width - cache.pane_w) / 2;
        cache.y0 = (image_height - cache.pane_h) / 2;

        cache.left_roi.assign(cache.roi_w * cache.roi_h * 3, 0);
        cache.right_roi.assign(cache.roi_w * cache.roi_h * 3, 0);

        const float focus_x = 0.64f * static_cast<float>(image_width);
        const float focus_y = 0.38f * static_cast<float>(image_height);
        const float source_span_x = 18.0f;
        const float source_span_y = source_span_x * (static_cast<float>(cache.roi_h) / static_cast<float>(cache.roi_w));

        for (int ry = 0; ry < cache.roi_h; ry++) {
            for (int rx = 0; rx < cache.roi_w; rx++) {
                float u = (static_cast<float>(rx) + 0.5f) / static_cast<float>(cache.roi_w);
                float v = (static_cast<float>(ry) + 0.5f) / static_cast<float>(cache.roi_h);

                float source_x = focus_x + (u - 0.5f) * source_span_x;
                float source_y = focus_y + (v - 0.5f) * source_span_y;

                if (source_x < 0.0f) source_x = 0.0f;
                if (source_x > static_cast<float>(image_width - 1)) source_x = static_cast<float>(image_width - 1);
                if (source_y < 0.0f) source_y = 0.0f;
                if (source_y > static_cast<float>(image_height - 1)) source_y = static_cast<float>(image_height - 1);

                color left_c = sample_color_at(scene, world, source_x, source_y, 1, false);
                color right_c = sample_color_at(scene, world, source_x, source_y, 100, true);

                write_u8_pixel(cache.left_roi, cache.roi_w, rx, ry, left_c);
                write_u8_pixel(cache.right_roi, cache.roi_w, rx, ry, right_c);
            }
        }
    }

    // Clear current output row to dark gray background.
    for (int i = 0; i < image_width; i++) {
        int idx = (j * image_width + i) * 3;
        pixels[idx + 0] = 16;
        pixels[idx + 1] = 16;
        pixels[idx + 2] = 16;
    }

    if (j >= cache.y0 && j < (cache.y0 + cache.pane_h)) {
        int roi_y = (j - cache.y0) / cache.scale;

        for (int dx = 0; dx < cache.pane_w; dx++) {
            int roi_x = dx / cache.scale;

            int left_dst_x = cache.left_x0 + dx;
            int right_dst_x = cache.right_x0 + dx;
            int roi_idx = (roi_y * cache.roi_w + roi_x) * 3;

            if (left_dst_x >= 0 && left_dst_x < image_width) {
                int out_idx = (j * image_width + left_dst_x) * 3;
                pixels[out_idx + 0] = cache.left_roi[roi_idx + 0];
                pixels[out_idx + 1] = cache.left_roi[roi_idx + 1];
                pixels[out_idx + 2] = cache.left_roi[roi_idx + 2];
            }

            if (right_dst_x >= 0 && right_dst_x < image_width) {
                int out_idx = (j * image_width + right_dst_x) * 3;
                pixels[out_idx + 0] = cache.right_roi[roi_idx + 0];
                pixels[out_idx + 1] = cache.right_roi[roi_idx + 1];
                pixels[out_idx + 2] = cache.right_roi[roi_idx + 2];
            }
        }
    }

    // Draw center separator line.
    if (half_width >= 0 && half_width < image_width) {
        int idx = (j * image_width + half_width) * 3;
        pixels[idx + 0] = 230;
        pixels[idx + 1] = 230;
        pixels[idx + 2] = 230;
    }
}

} // namespace

render_scene create_image6_2_scene() {
    render_scene scene;
    scene.cam.aspect_ratio = 16.0f / 9.0f;
    scene.cam.image_width = 800;
    // Note: samples_per_pixel is not used directly; it's handled in fill_scanline_comparison
    scene.cam.samples_per_pixel = 1;

    auto world = std::make_shared<hittable_list>();
    world->add(make_shared<sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f));
    world->add(make_shared<sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f));

    scene.world = world;
    scene.ray_color = image6_2_ray_color;
    scene.fill_scanline_override = fill_scanline_comparison;
    
    return scene;
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image6-2 (Comparison)", create_image6_2_scene}); }
} auto_reg;
} // namespace

} // namespace image_render
