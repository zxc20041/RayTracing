#ifndef IMAGE_RENDER_H
#define IMAGE_RENDER_H

#include <cstdint>
#include <vector>

namespace image_render {

struct image_spec {
    float aspect_ratio;
    int image_width;
};

struct render_entry {
    const char* name;
    image_spec (*spec)();
    void (*fill_scanline)(int j, int image_width, int image_height, std::vector<uint8_t>& pixels);
};

// Register a renderer. Called automatically by each imageN.cpp at startup.
void register_renderer(const render_entry& entry);

// Returns all registered renderers in registration order.
const std::vector<render_entry>& get_renderers();

// Returns the default render setup for image2.
image_spec image2_spec();

// Fills one scanline of the pixel buffer for image2.
void fill_image2_scanline(int j, int image_width, int image_height, std::vector<uint8_t>& pixels);

// Returns the default render setup for image3.
image_spec image3_spec();

// Fills one scanline of the pixel buffer for image3.
void fill_image3_scanline(int j, int image_width, int image_height, std::vector<uint8_t>& pixels);

// Returns the default render setup for image4.
image_spec image4_spec();

// Fills one scanline of the pixel buffer for image4.
void fill_image4_scanline(int j, int image_width, int image_height, std::vector<uint8_t>& pixels);

} // namespace image_render

#endif
