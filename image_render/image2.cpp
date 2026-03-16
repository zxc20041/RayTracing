#include "image_render.h"

#include "ray.h"
#include "vec3.h"

namespace image_render {

namespace {

using color = vec3;

color ray_color(const ray& r) {
	vec3 unit_direction = unit_vector(r.direction());
	auto a = 0.5f * (unit_direction.y() + 1.0f);
	return (1.0f - a) * color(1.0f, 1.0f, 1.0f) + a * color(0.5f, 0.7f, 1.0f);
}

} // namespace

image_spec image2_spec() {
	return {16.0f / 9.0f, 800};
}

void fill_image2_scanline(int j, int image_width, int image_height, std::vector<uint8_t>& pixels) {
	auto focal_length = 1.0f;
	auto viewport_height = 2.0f;
	auto viewport_width = viewport_height * (static_cast<float>(image_width) / image_height);
	auto camera_center = point3(0, 0, 0);

	auto viewport_u = vec3(viewport_width, 0, 0);
	auto viewport_v = vec3(0, -viewport_height, 0);

	auto pixel_delta_u = viewport_u / image_width;
	auto pixel_delta_v = viewport_v / image_height;

	auto viewport_upper_left = camera_center
							 - vec3(0, 0, focal_length) - viewport_u / 2 - viewport_v / 2;
	auto pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);

	for (int i = 0; i < image_width; i++) {
		auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
		auto ray_direction = pixel_center - camera_center;
		ray r(camera_center, ray_direction);

		color pixel_color = ray_color(r);

		int idx = (j * image_width + i) * 3;
		pixels[idx + 0] = static_cast<uint8_t>(255.999f * pixel_color.x());
		pixels[idx + 1] = static_cast<uint8_t>(255.999f * pixel_color.y());
		pixels[idx + 2] = static_cast<uint8_t>(255.999f * pixel_color.z());
	}
}

namespace {
struct auto_reg_t {
    auto_reg_t() { register_renderer({"image2", image2_spec, fill_image2_scanline}); }
} auto_reg;
} // namespace

} // namespace image_render
