#ifndef CAMERA_H
#define CAMERA_H

#include <cstdint>
#include <iostream>
#include <vector>
#include "rtweekend.h"
#include "hittable.h"
#include "material.h"

class camera {
  public:
    using ray_color_fn = color (*)(const ray&, const hittable&);
        using ray_color_depth_fn = color (*)(const ray&, const hittable&, int);

    float  aspect_ratio = 1.0f;
    int    image_width = 100;
    int    samples_per_pixel = 10;   // Count of random samples for each pixel
    int    max_depth         = 10;   // Maximum number of ray bounces into scene
    bool   enable_gamma_correction = false;

    float vfov = 90;  // Vertical view angle (field of view)

    void set_ray_color_fn(ray_color_fn fn) {
        shade_fn = fn;
        shade_with_depth_fn = nullptr;
    }

    void set_ray_color_fn(ray_color_depth_fn fn) {
        shade_with_depth_fn = fn;
        shade_fn = nullptr;
    }

    void begin_render() {
        initialize();
    }

    int image_height() const {
        return image_height_value;
    }

    void fill_scanline(int j, const hittable& world, std::vector<uint8_t>& pixels) const {
        for (int i = 0; i < image_width; i++) {
            color pixel_color(0.0f, 0.0f, 0.0f);
            for (int sample = 0; sample < samples_per_pixel; sample++) {
                ray r = get_ray(i, j);
                pixel_color += shade(r, world);
            }

            pixel_color *= pixel_samples_scale;

            static const interval intensity(0.000f, 0.999f);
            float r = pixel_color.x();
            float g = pixel_color.y();
            float b = pixel_color.z();

            if (enable_gamma_correction) {
                r = linear_to_gamma(r);
                g = linear_to_gamma(g);
                b = linear_to_gamma(b);
            }

            int idx = (j * image_width + i) * 3;
            pixels[idx + 0] = static_cast<uint8_t>(256.0f * intensity.clamp(r));
            pixels[idx + 1] = static_cast<uint8_t>(256.0f * intensity.clamp(g));
            pixels[idx + 2] = static_cast<uint8_t>(256.0f * intensity.clamp(b));
        }
    }

    void render(const hittable& world) {
        begin_render();

        std::cout << "P3\n" << image_width << ' ' << image_height_value << "\n255\n";

        for (int j = 0; j < image_height_value; j++) {
            std::clog << "\rScanlines remaining: " << (image_height_value - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                color pixel_color(0.0f, 0.0f, 0.0f);
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    ray r = get_ray(i, j);
                    pixel_color += shade(r, world);
                }

                pixel_color *= pixel_samples_scale;

                float r = pixel_color.x();
                float g = pixel_color.y();
                float b = pixel_color.z();

                if (enable_gamma_correction) {
                    r = linear_to_gamma(r);
                    g = linear_to_gamma(g);
                    b = linear_to_gamma(b);
                }

                static const interval intensity(0.000f, 0.999f);
                int rbyte = static_cast<int>(256.0f * intensity.clamp(r));
                int gbyte = static_cast<int>(256.0f * intensity.clamp(g));
                int bbyte = static_cast<int>(256.0f * intensity.clamp(b));
                std::cout << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
            }
        }

        std::clog << "\rDone.                 \n";
    }

    // Public methods for custom rendering logic
    ray get_ray(float i, float j, bool jitter = true) const {
        // Build a camera ray for pixel-space coordinate (i, j).
        auto offset = jitter ? sample_square() : vec3(0.0f, 0.0f, 0.0f);
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        auto ray_origin = center;
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    ray get_ray(int i, int j) const {
        return get_ray(static_cast<float>(i), static_cast<float>(j), true);
    }

    color shade(const ray& r, const hittable& world) const {
        if (shade_with_depth_fn) {
            return shade_with_depth_fn(r, world, max_depth);
        }

        if (shade_fn) {
            return shade_fn(r, world);
        }

        return default_ray_color(r, world);
    }

    private:
        int image_height_value = 1;
        float pixel_samples_scale;  // Color scale factor for a sum of pixel samples
        point3 center;
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        ray_color_fn shade_fn = nullptr;
        ray_color_depth_fn shade_with_depth_fn = nullptr;
        float focal_length = 1.0f;
        float theta = degrees_to_radians(vfov);
        float h = std::tan(theta/2);
        float viewport_height = 2 * h * focal_length;

        static color default_ray_color(const ray& r, const hittable&) {
                vec3 unit_direction = unit_vector(r.direction());
                auto a = 0.5f * (unit_direction.y() + 1.0f);
                return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
        }

    void initialize() {
        image_height_value = int(image_width / aspect_ratio);
        image_height_value = (image_height_value < 1) ? 1 : image_height_value;
        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = point3(0, 0, 0);

        
        auto viewport_height = 2.0f;
        auto viewport_width = viewport_height * (static_cast<float>(image_width) / image_height_value);

        auto viewport_u = vec3(viewport_width, 0, 0);
        auto viewport_v = vec3(0, -viewport_height, 0);

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height_value;

        auto viewport_upper_left = center - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);
    }

    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_float() - 0.5, random_float() - 0.5, 0);
    }
};

#endif
