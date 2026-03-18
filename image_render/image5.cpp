#include "image_render_base/rtweekend.h"
#include "image_render_base/hittable.h"
#include "image_render_base/hittable_list.h"
#include "image_render_base/sphere.h"
#include "image_render.h"

namespace image_render {
namespace {
using color = vec3;

float hit_sphere(const point3& center, float radius, const ray& r) {
    vec3 oc = center - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), oc);
    auto c = oc.length_squared() - radius*radius;
    auto discriminant = h*h - a*c;

    if (discriminant < 0) {
        return -1.0;
    } else {
        return (h - std::sqrt(discriminant)) / a;
    }
}

color ray_color(const ray& r, const hittable& world) {
    hit_record rec;
    if (world.hit(r, 0, infinity, rec)) {
        return 0.5 * (rec.normal + color(1,1,1));
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.y() + 1.0);
    return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
}
}

image_spec image5_spec() {
    return {16.0f / 9.0f, 800};
}

void fill_image5_scanline(int j, int image_width, int image_height, std::vector<uint8_t>& pixels) {
    // World

    hittable_list world;

    world.add(make_shared<sphere>(point3(0.0f, 0.0f, -1.0f), 0.5f));
    world.add(make_shared<sphere>(point3(0.0f, -100.5f, -1.0f), 100.0f));

    // Camera

    auto focal_length = 1.0f;
    auto viewport_height = 2.0f;
    auto viewport_width = viewport_height * (static_cast<float>(image_width)/image_height);
    auto camera_center = point3(0, 0, 0);

    // Calculate the vectors across the horizontal and down the vertical viewport edges.
    auto viewport_u = vec3(viewport_width, 0.0f, 0.0f);
    auto viewport_v = vec3(0.0f, -viewport_height, 0.0f);

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.
    auto pixel_delta_u = viewport_u / image_width;
    auto pixel_delta_v = viewport_v / image_height;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = camera_center
                             - vec3(0.0f, 0.0f, focal_length) - viewport_u/2.0f - viewport_v/2.0f;
    auto pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

    // Render
    
    for (int i = 0; i < image_width; i++) {
        auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        auto ray_direction = pixel_center - camera_center;
        ray r(camera_center, ray_direction);

        color pixel_color = ray_color(r, world);

        int idx = (j * image_width + i) * 3;
        pixels[idx + 0] = static_cast<uint8_t>(255.999f * pixel_color.x());
        pixels[idx + 1] = static_cast<uint8_t>(255.999f * pixel_color.y());
        pixels[idx + 2] = static_cast<uint8_t>(255.999f * pixel_color.z());
    }
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image5", image5_spec, fill_image5_scanline}); }
} auto_reg;
} // namespace

} // namespace image_render