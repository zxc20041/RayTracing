#include "image_render/image_render.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <SDL.h>

namespace {

enum class app_action {
    none,
    next,
    previous,
    quit,
};

app_action poll_action(bool wait_for_event) {
    SDL_Event event;

    if (wait_for_event) {
        if (!SDL_WaitEvent(&event)) {
            return app_action::quit;
        }
    } else {
        if (!SDL_PollEvent(&event)) {
            return app_action::none;
        }
    }

    do {
        if (event.type == SDL_QUIT) {
            return app_action::quit;
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                return app_action::next;
            }

            if (event.button.button == SDL_BUTTON_RIGHT) {
                return app_action::previous;
            }
        }
    } while (SDL_PollEvent(&event));

    return app_action::none;
}

bool ensure_texture(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture*& texture, int image_width, int image_height) {
    SDL_SetWindowSize(window, image_width, image_height);

    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        image_width,
        image_height);

    return texture != nullptr;
}

app_action render_image(
    SDL_Window* window,
    SDL_Renderer* renderer,
    SDL_Texture*& texture,
    const image_render::render_entry& entry,
    std::vector<uint8_t>& pixels) {
    const auto spec = entry.spec();
    const auto aspect_ratio = spec.aspect_ratio;
    const int image_width = spec.image_width;
    int image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    if (!ensure_texture(window, renderer, texture, image_width, image_height)) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << '\n';
        return app_action::quit;
    }

    SDL_SetWindowTitle(window, entry.name);
    pixels.assign(image_width * image_height * 3, 0);

    for (int j = 0; j < image_height; j++) {
        std::clog << "\r[" << entry.name << "] Scanlines remaining: " << (image_height - j) << ' ' << std::flush;

        app_action action = poll_action(false);
        if (action != app_action::none) {
            return action;
        }

        entry.fill_scanline(j, image_width, image_height, pixels);

        SDL_UpdateTexture(texture, nullptr, pixels.data(), image_width * 3);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    std::clog << "\r[" << entry.name << "] Done.                 \n";
    return app_action::none;
}

} // namespace

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    const auto& renderers = image_render::get_renderers();
    int current_renderer = static_cast<int>(renderers.size()) - 1;

    // SDL2 init
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
        return 1;
    }

    const auto initial_spec = renderers[current_renderer].spec();
    int initial_height = int(initial_spec.image_width / initial_spec.aspect_ratio);
    initial_height = (initial_height < 1) ? 1 : initial_height;

    SDL_Window* window = SDL_CreateWindow(
        "RayTracing",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        initial_spec.image_width, initial_height, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = nullptr;
    std::vector<uint8_t> pixels;

    while (true) {
        app_action action = render_image(window, renderer, texture, renderers[current_renderer], pixels);
        if (action == app_action::quit) {
            break;
        }

        if (action == app_action::next) {
            current_renderer = (current_renderer + 1) % static_cast<int>(renderers.size());
            continue;
        }

        if (action == app_action::previous) {
            current_renderer = (current_renderer + static_cast<int>(renderers.size()) - 1) % static_cast<int>(renderers.size());
            continue;
        }

        while (true) {
            action = poll_action(true);
            if (action == app_action::quit) {
                current_renderer = -1;
                break;
            }

            if (action == app_action::next) {
                current_renderer = (current_renderer + 1) % static_cast<int>(renderers.size());
                break;
            }

            if (action == app_action::previous) {
                current_renderer = (current_renderer + static_cast<int>(renderers.size()) - 1) % static_cast<int>(renderers.size());
                break;
            }
        }

        if (current_renderer < 0) {
            break;
        }
    }

    if (texture) {
        SDL_DestroyTexture(texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}