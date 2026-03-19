#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "color.h"

#include <cstdint>
#include <iostream>
#include <vector>

class camera {
  public:
    using ray_color_fn = color (*)(const ray&, const hittable&);

    float aspect_ratio = 1.0f;
    int image_width = 100;

    void set_ray_color_fn(ray_color_fn fn) {
        shade_fn = fn;
    }

    void begin_render() {
        initialize();
    }

    int image_height() const {
        return image_height_value;
    }

    void fill_scanline(int j, const hittable& world, std::vector<uint8_t>& pixels) const {
        for (int i = 0; i < image_width; i++) {
            auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
            auto ray_direction = pixel_center - center;
            ray r(center, ray_direction);

            color pixel_color = shade(r, world);

            int idx = (j * image_width + i) * 3;
            pixels[idx + 0] = static_cast<uint8_t>(255.999f * pixel_color.x());
            pixels[idx + 1] = static_cast<uint8_t>(255.999f * pixel_color.y());
            pixels[idx + 2] = static_cast<uint8_t>(255.999f * pixel_color.z());
        }
    }

    void render(const hittable& world) {
        begin_render();

        std::cout << "P3\n" << image_width << ' ' << image_height_value << "\n255\n";

        for (int j = 0; j < image_height_value; j++) {
            std::clog << "\rScanlines remaining: " << (image_height_value - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
                auto ray_direction = pixel_center - center;
                ray r(center, ray_direction);

                color pixel_color = shade(r, world);
                write_color(std::cout, pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

  private:
    int image_height_value = 1;
    point3 center;
    point3 pixel00_loc;
    vec3 pixel_delta_u;
    vec3 pixel_delta_v;
    ray_color_fn shade_fn = nullptr;

    static color default_ray_color(const ray& r, const hittable&) {
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5f * (unit_direction.y() + 1.0f);
        return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
    }

    color shade(const ray& r, const hittable& world) const {
        if (shade_fn) {
            return shade_fn(r, world);
        }

        return default_ray_color(r, world);
    }

    void initialize() {
        image_height_value = int(image_width / aspect_ratio);
        image_height_value = (image_height_value < 1) ? 1 : image_height_value;

        center = point3(0, 0, 0);

        auto focal_length = 1.0f;
        auto viewport_height = 2.0f;
        auto viewport_width = viewport_height * (static_cast<float>(image_width) / image_height_value);

        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height_value;

        auto viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);
    }
};

#endif
