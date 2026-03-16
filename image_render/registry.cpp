#include "image_render.h"

#include <vector>

namespace image_render {

namespace {

std::vector<render_entry>& registry() {
    static std::vector<render_entry> r;
    return r;
}

} // namespace

void register_renderer(const render_entry& entry) {
    registry().push_back(entry);
}

const std::vector<render_entry>& get_renderers() {
    return registry();
}

} // namespace image_render
