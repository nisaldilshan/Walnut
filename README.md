# Walnut

Walnut is a small application framework for building desktop tools with [Dear ImGui](https://github.com/ocornut/imgui) (docking branch). You write your app as a stack of layers; Walnut handles the window, the render loop and the ImGui setup.

The renderer and windowing system are picked at build time:

- **Renderer:** Vulkan, OpenGL 4.1 or WebGPU (Dawn)
- **Windowing system:** GLFW or SDL3

This is a fork of [TheCherno/Walnut](https://github.com/TheCherno/Walnut), reworked to support several graphics APIs and to take its dependencies from Conan or vcpkg.

## Supported configurations

Each build uses exactly one renderer and one windowing system. Dependencies come from either Conan 2 or vcpkg:

| Renderer | Windowing system | Conan | vcpkg |
|---|---|---|---|
| Vulkan | GLFW | ✅ (default) | ✅ (default) |
| Vulkan | SDL3 | ✅ | ✅ |
| OpenGL | GLFW | ✅ | ✅ |
| OpenGL | SDL3 | ✅ | ✅ |
| WebGPU | GLFW | ✅ | ❌ not supported |
| WebGPU | SDL3 | ❌ fails to build | ❌ not supported |

✅ means the library and the example app build, on Windows (MSVC) and Linux (GCC). CI builds every ✅ combination on each pull request (see `.github/workflows/ci.yml`). On Linux, CI also runs the [render test](#tests) for each of them, on Mesa's software renderers since the runners have no GPU.

In the browser, Walnut runs with GLFW and either WebGL 2 (the OpenGL renderer) or WebGPU, built with Conan and Emscripten. CI builds both, and every push to `master` publishes them to GitHub Pages, see [Building for the browser](#building-for-the-browser).

- **Other platforms.** The build files also have code paths for macOS, Android and iOS, but these are not verified at the moment. On macOS, Vulkan runs through MoltenVK. vcpkg does not provide MoltenVK, so install it separately (e.g. `brew install molten-vk`).
- **Linux system packages.** With vcpkg, some ports expect build tools and X11/Wayland headers from the system. The "Install system packages" step in `.github/workflows/ci.yml` lists the apt packages that are needed on Ubuntu. With Conan, the recipes can install them for you when you pass `-c tools.system.package_manager:mode=install`.
- **WebGPU with vcpkg.** Walnut's WebGPU backend downloads its own Dawn build, and imgui's WebGPU backend has to be compiled from Conan's imgui sources. vcpkg's imgui only works with vcpkg's own (newer) Dawn. CMake stops with an error if you try this combination.
- **WebGPU + SDL3 with Conan.** The downloaded `sdl3webgpu` helper does not compile against the downloaded Dawn version.

## Requirements

- CMake 3.22 or newer and a C++17 compiler
- [Conan 2](https://conan.io) or [vcpkg](https://vcpkg.io)
- A GPU driver with Vulkan or OpenGL 4.1 support. The Vulkan SDK is not needed, because the Vulkan loader comes from the package manager.

## Building with Conan

Choose the configuration with the `rendering_backend` (`Vulkan`, `OpenGL`, `WebGPU`) and `windowing_system` (`GLFW`, `SDL`) options:

```
conan install . --build=missing -s build_type=Debug -o "&:rendering_backend=Vulkan" -o "&:windowing_system=GLFW"
cmake --preset conan-default
cmake --build --preset conan-debug
```

`conan-default` is the configure preset Conan generates for multi-config generators such as Visual Studio. With a single-config generator (Makefiles, Ninja), configure with `cmake --preset conan-debug` instead.

The WebGPU renderer downloads Dawn and its helper libraries at configure time into `external_deps/`.

## Building with vcpkg

Pass vcpkg's toolchain file and choose the configuration with `RENDERER` (`Vulkan`, `OpenGL`) and `WINDOWING_SYSTEM` (`GLFW`, `SDL`). Replace `<vcpkg-root>` with the path to your vcpkg checkout:

```
cmake -B build-vcpkg/vulkan-glfw -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>/scripts/buildsystems/vcpkg.cmake -DRENDERER=Vulkan -DWINDOWING_SYSTEM=GLFW
cmake --build build-vcpkg/vulkan-glfw --config Debug
```

vcpkg installs the dependencies listed in `vcpkg.json` during the first configure. Which of its features get installed follows from `RENDERER` and `WINDOWING_SYSTEM`. Use a separate build folder for each configuration.

## Building for the browser

Use Conan with the Emscripten profile in `testing/conan-profiles/emscripten`, which also downloads the Emscripten SDK. Pick `OpenGL` for WebGL 2 or `WebGPU`; the windowing system has to be `GLFW`:

```
conan build . --build=missing -pr:h testing/conan-profiles/emscripten -pr:b default -s build_type=Release -o "&:rendering_backend=WebGPU" -o "&:windowing_system=GLFW"
python -m http.server 8000 --directory build/Release/WalnutApp
```

The example app is written to `build/Release/WalnutApp/` as `index.html`, `index.js` and `index.wasm`, which any static web server can serve; open http://localhost:8000. WebGPU needs a browser that supports it, such as current Chrome or Edge.

The `pages` job in `.github/workflows/ci.yml` publishes both builds from the latest `master` to GitHub Pages: the landing page (`.github/pages/index.html`) links to `webgl/` and `webgpu/`. For the job to deploy, set **Settings → Pages → Source** to **GitHub Actions** in the repository.

## The example app

`WalnutApp/` contains an example that draws a viewport with a random-noise image, a settings panel and the ImGui demo window. It is built as `App` along with the library. Set `-DWALNUT_BUILD_EXAMPLES=OFF` to skip it.

ImGui saves the window layout to `imgui.ini` in the working directory and restores it on the next start. When there is no saved layout (on the first start, and always in the browser), the app docks its windows with `SetDefaultDockLayoutCallback`: the viewport on the left, the settings on the top right and the ImGui demo below them.

## Using Walnut in your project

### With vcpkg

`ports/walnut` is a vcpkg port. Add it to your project as an overlay port in `vcpkg-configuration.json`:

```json
{
  "overlay-ports": [ "path/to/Walnut/ports" ]
}
```

Then depend on it in `vcpkg.json`. The default features are `vulkan` and `glfw`. To use `opengl` or `sdl` instead, turn the default features off:

```json
{
  "dependencies": [
    { "name": "walnut", "default-features": false, "features": [ "opengl", "glfw" ] }
  ]
}
```

And in CMake:

```cmake
find_package(Walnut CONFIG REQUIRED)
target_link_libraries(main PRIVATE Walnut::walnut)
```

`Walnut::walnut` brings in the include paths, imgui, and the defines the headers need (`RENDERER_BACKEND`, `USE_SDL`).

### With Conan

Walnut is not on ConanCenter. `conan create .` builds the package into your local cache from the `v<version>` git tag, using the same options as above. Then add `walnut/<version>` to your project's requirements.

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

- `SetDefaultDockLayoutCallback(callback)`: arranges the windows in the main dockspace when there is no saved layout. The callback gets the dockspace's ID; split it and dock windows into it with ImGui's `DockBuilder` functions from `imgui_internal.h`, as `WalnutApp/src/WalnutApp.cpp` does.
- `CaptureNextFrame(callback)`: reads back the next frame rendered to the main window, as RGBA pixels with the top row first. Works with every renderer on the desktop. In the browser it is untested with WebGL, and the WebGPU renderer passes a null pointer instead of pixels.

## Tests

`tests/render/` is a program that opens a window, draws a `Walnut::Image` and a rectangle with ImGui, reads the frame back with `CaptureNextFrame` and checks the colors of the pixels. It then changes the image with `SetData` and checks again. It is built with the library unless `-DWALNUT_BUILD_TESTS=OFF` is set, and runs through CTest:

```
ctest --test-dir build/Release --output-on-failure    # single-config generators
ctest --test-dir build -C Release --output-on-failure # Visual Studio
```

The test needs a display and a GPU driver. It saves each frame it checks as `render-test-<renderer>-<windowing>-<phase>.png` in its working directory. With the Vulkan renderer it asks for the validation layer, and any validation message fails the test. The Vulkan loader 1.3.239 from Conan skips the layer from Conan, though, because it misreads the `api_version` in the layer's manifest, so in practice validation is only on when the loader finds another copy of the layer (e.g. from the Vulkan SDK). On Linux, CI runs it on a virtual X display (Xvfb) with Mesa's software renderers, llvmpipe for OpenGL and lavapipe for Vulkan and WebGPU (Dawn runs on Vulkan), through `.github/scripts/render-test.sh`. Windows runners have no GPU driver for Vulkan or OpenGL 4.1, so there the test is built but not run.

`tests/package/` is a minimal app that CI builds against the Walnut package installed with vcpkg.

## Repository layout

| Path | Contents |
|---|---|
| `Walnut/src/Walnut/` | The library: application, layers, images, input |
| `Walnut/src/Walnut/GraphicsAPI/` | One folder per renderer (Vulkan, OpenGL, WebGPU) |
| `WalnutApp/` | Example application |
| `conanfile.py` | Conan recipe |
| `vcpkg.json` | vcpkg manifest for building Walnut itself |
| `ports/walnut/` | vcpkg port for consumers |
| `tests/render/` | Render test: draws a known scene and checks the pixels read back from the window |
| `tests/package/` | Minimal app that CI builds against the installed Walnut package |
| `.github/workflows/ci.yml` | CI: builds every supported combination with Conan and vcpkg on Windows and Linux, and for the browser; runs the render test on Linux; publishes the browser builds to GitHub Pages |
| `.github/scripts/` | Scripts the CI jobs run: the vcpkg build and the render test under Xvfb |
| `.github/pages/` | Landing page of the GitHub Pages site |
| `testing/conan-profiles/emscripten` | Conan profile for the browser builds |
| `cmake/` | CMake modules: build options, dependency lookup (`WalnutConan.cmake`, `WalnutVcpkg.cmake`), install rules, package config template |

The Premake files (`premake5.lua`, `WalnutExternal.lua`, `scripts/Setup.bat`) and the other profiles in `testing/conan-profiles/` come from earlier versions of the project and are not maintained.

## Releasing

The version number appears in `conanfile.py`, `CMakeLists.txt` (`project(... VERSION)`), `vcpkg.json` and `ports/walnut/vcpkg.json`. To release:

1. Update the version in all four files.
2. Tag the release as `v<version>`.
3. In `ports/walnut/portfile.cmake`, set `SHA512` to the hash vcpkg reports for the new tag. Setting it to `0` and installing the port once makes vcpkg print the correct value.

## Third-party libraries

- [Dear ImGui](https://github.com/ocornut/imgui) (docking branch)
- [GLFW](https://github.com/glfw/glfw) or [SDL3](https://github.com/libsdl-org/SDL)
- [Vulkan loader](https://github.com/KhronosGroup/Vulkan-Loader), [glad](https://github.com/Dav1dde/glad) for OpenGL, or [Dawn](https://dawn.googlesource.com/dawn) via [WebGPU-distribution](https://github.com/eliemichel/WebGPU-distribution) for WebGPU
- [stb_image](https://github.com/nothings/stb)
- Walnut embeds the [Roboto](https://fonts.google.com/specimen/Roboto) font ([Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0))

## License

MIT, see [LICENSE.txt](LICENSE.txt).
