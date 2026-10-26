# Walnut

Walnut is a small framework for building desktop tools with [Dear ImGui](https://github.com/ocornut/imgui) (docking branch). You write your app as a stack of layers, and Walnut takes care of the window, the render loop and the ImGui setup.

You choose the renderer (Vulkan, OpenGL or WebGPU) and the windowing system (GLFW or SDL3) at build time.

This is a fork of [TheCherno/Walnut](https://github.com/TheCherno/Walnut) that supports several graphics APIs and gets its dependencies from Conan or vcpkg.

## Supported configurations

Each build uses one renderer and one windowing system, with dependencies from either Conan 2 or vcpkg:

| Renderer | Windowing system | Conan | vcpkg |
|---|---|---|---|
| Vulkan | GLFW | ✅ (default) | ✅ (default) |
| Vulkan | SDL3 | ✅ | ✅ |
| OpenGL | GLFW | ✅ | ✅ |
| OpenGL | SDL3 | ✅ | ✅ |
| WebGPU | GLFW | ✅ | ❌ not supported |
| WebGPU | SDL3 | ❌ fails to build | ❌ not supported |

In the browser, Walnut runs with GLFW and either WebGL 2 (the OpenGL renderer) or WebGPU, built with Conan and Emscripten. CI builds both and publishes them to GitHub Pages on every push to `master`. See [Building for the browser](#building-for-the-browser).

Some things to know:

- The build files have code paths for macOS, Android and iOS, but they are untested. On macOS, Vulkan runs through MoltenVK. vcpkg does not provide it, so install it yourself (for example with `brew install molten-vk`).
- With vcpkg on Linux, some ports expect build tools and X11/Wayland headers from the system. The "Install system packages" step in `.github/workflows/ci.yml` lists the apt packages Ubuntu needs. With Conan, pass `-c tools.system.package_manager:mode=install` and the recipes install them for you.
- Walnut's WebGPU backend downloads its own Dawn build, and imgui's WebGPU backend has to be compiled from Conan's imgui sources. vcpkg's imgui only works with vcpkg's own, newer Dawn, so CMake stops with an error for WebGPU with vcpkg.
- For WebGPU with SDL3 and Conan, the downloaded `sdl3webgpu` helper does not compile against the downloaded Dawn version.

## Requirements

- CMake 3.22 or newer and a C++17 compiler
- [Conan 2](https://conan.io) or [vcpkg](https://vcpkg.io)
- A GPU driver with Vulkan or OpenGL 4.1 support. You don't need the Vulkan SDK, since the package manager supplies the Vulkan loader.

## Building with Conan

Pick the configuration with the `rendering_backend` (`Vulkan`, `OpenGL`, `WebGPU`) and `windowing_system` (`GLFW`, `SDL`) options:

```
conan install . --build=missing -s build_type=Debug -o "&:rendering_backend=Vulkan" -o "&:windowing_system=GLFW"
cmake --preset conan-default
cmake --build --preset conan-debug
```

`conan-default` is the configure preset Conan generates for multi-config generators such as Visual Studio. With a single-config generator (Makefiles, Ninja), configure with `cmake --preset conan-debug` instead.

The WebGPU renderer downloads Dawn and its helper libraries into `external_deps/` at configure time.

## Building with vcpkg

Pass vcpkg's toolchain file and set `RENDERER` (`Vulkan`, `OpenGL`) and `WINDOWING_SYSTEM` (`GLFW`, `SDL`). Replace `<vcpkg-root>` with the path to your vcpkg checkout:

```
cmake -B build-vcpkg/vulkan-glfw -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake -DRENDERER=Vulkan -DWINDOWING_SYSTEM=GLFW
cmake --build build-vcpkg/vulkan-glfw --config Debug
```

vcpkg installs the dependencies listed in `vcpkg.json` on the first configure, and `RENDERER` and `WINDOWING_SYSTEM` decide which features it installs. Use a separate build folder for each configuration.

## Building for the browser

Use Conan with the Emscripten profile in `testing/conan-profiles/emscripten`, which also downloads the Emscripten SDK. Pick `OpenGL` for WebGL 2 or `WebGPU`. The windowing system must be `GLFW`:

```
conan build . --build=missing -pr:h testing/conan-profiles/emscripten -pr:b default -s build_type=Release -o "&:rendering_backend=WebGPU" -o "&:windowing_system=GLFW"
python -m http.server 8000 --directory build/Release/WalnutApp
```

The example app ends up in `build/Release/WalnutApp/` as `index.html`, `index.js` and `index.wasm`, which any static web server can serve. WebGPU needs a browser that supports it, such as current Chrome or Edge.

To see a demo, go to https://nisaldilshan.github.io/Walnut/.

## The example app

`WalnutApp/` is an example that draws a viewport with a random-noise image, a settings panel and the ImGui demo window. It is built as `App` along with the library; set `-DWALNUT_BUILD_EXAMPLES=OFF` to skip it.

ImGui saves the window layout to `imgui.ini` in the working directory and restores it on the next start. When there is no saved layout (on the first start, and always in the browser), the app docks its windows through `SetDefaultDockLayoutCallback`, with the viewport on the left, the settings on the top right and the ImGui demo below them.

## Using Walnut in your project

### With vcpkg

`ports/walnut` is a vcpkg port. To use it, add it as an overlay port in `vcpkg-configuration.json`:

```json
{
  "overlay-ports": [ "path/to/Walnut/ports" ]
}
```

Then depend on it in `vcpkg.json`. The default features are `vulkan` and `glfw`; turn them off to get `opengl` or `sdl` instead:

```json
{
  "dependencies": [
    { "name": "walnut", "default-features": false, "features": [ "opengl", "glfw" ] }
  ]
}
```

In CMake:

```cmake
find_package(Walnut CONFIG REQUIRED)
target_link_libraries(main PRIVATE Walnut::walnut)
```

`Walnut::walnut` brings in the include paths, imgui and the defines the headers need (`RENDERER_BACKEND`, `USE_SDL`).

### With Conan

Walnut is not on ConanCenter. `conan create .` builds the package from the `v<version>` git tag into your local cache, with the same options as above. Then add `walnut/<version>` to your project's requirements.

### A minimal app

```cpp
#include <Walnut/Application.h>
#include <Walnut/EntryPoint.h>

#include <imgui.h>

class HelloLayer : public Walnut::Layer
{
public:
    void OnUIRender() override
    {
        ImGui::Begin("Hello");
        ImGui::Text("Hello from Walnut");
        ImGui::End();
    }
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
    Walnut::ApplicationSpecification spec;
    spec.Name = "My App";

    auto* app = new Walnut::Application(spec);
    app->PushLayer<HelloLayer>();
    return app;
}
```

`EntryPoint.h` provides `main()`, so include it in exactly one source file. Layers can override `OnAttach`, `OnDetach`, `OnUpdate(float ts)` and `OnUIRender`. `Walnut::Image` uploads pixel data to a texture that you can draw with `ImGui::Image`.

`Application` also has:

- `SetDefaultDockLayoutCallback(callback)` arranges the windows in the main dockspace when there is no saved layout. The callback receives the dockspace's ID. Split it and dock windows into it with ImGui's `DockBuilder` functions from `imgui_internal.h`, as `WalnutApp/src/WalnutApp.cpp` does.
- `CaptureNextFrame(callback)` reads back the next frame rendered to the main window, as RGBA pixels with the top row first. It works with every renderer on the desktop. In the browser, WebGL is untested and the WebGPU renderer passes a null pointer instead of pixels.

## Third-party libraries

- [Dear ImGui](https://github.com/ocornut/imgui) (docking branch)
- [GLFW](https://github.com/glfw/glfw) or [SDL3](https://github.com/libsdl-org/SDL)
- [Vulkan loader](https://github.com/KhronosGroup/Vulkan-Loader), [glad](https://github.com/Dav1dde/glad) for OpenGL, or [Dawn](https://dawn.googlesource.com/dawn) via [WebGPU-distribution](https://github.com/eliemichel/WebGPU-distribution) for WebGPU
- [stb_image](https://github.com/nothings/stb)
- Walnut embeds the [Roboto](https://fonts.google.com/specimen/Roboto) font ([Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0))

## License

MIT, see [LICENSE.txt](LICENSE.txt).
