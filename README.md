# RayTracing

A ray tracer built by following [_Ray Tracing in One Weekend_](https://github.com/RayTracing/raytracing.github.io).

## Dependencies

- C++17
- CMake 3.16+
- [SDL2](https://www.libsdl.org/) (via vcpkg, located in `vcpkg/`)

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

Or press **F5** in VS Code to build (Debug) and launch directly.

## Controls

| Input | Action |
|---|---|
| Left click | Next image |
| Right click | Previous image |
| Close window | Quit |

## Image sequence

| Name | Description |
|---|---|
| image2 | Gradient background |
| image3 | Red sphere |
| image4 | Normal-mapped sphere |

## Adding a new image

1. Create `image_render/imageN.cpp` and implement `imageN_spec()` + `fill_imageN_scanline()`.
2. Add a self-registration block at the end of the file (copy the pattern from `image4.cpp`).
3. Declare the two functions in `image_render/image_render.h`.
4. Add `image_render/imageN.cpp` to `CMakeLists.txt`.
