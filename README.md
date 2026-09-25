# calendar-engine

A C++20 OpenGL project built with GLFW, currently working through [LearnOpenGL](https://learnopengl.com). It builds for desktop OpenGL 3.3 core (with glad) or OpenGL ES 3.1, and targets a Raspberry Pi 4 kiosk as well as the dev container.

## Requirements

- Docker
- VS Code with the Dev Containers extension

Everything else (compiler, CMake, Ninja, X11, Wayland, EGL/GLES and Mesa dev packages) is installed in the dev container. See `.devcontainer/Dockerfile`.

## Getting started

1. Open the folder in VS Code and run **Dev Containers: Reopen in Container**.
2. Build inside the container with one of the presets:

   | Preset | GL API | Window | Build type |
   |---|---|---|---|
   | `dev-core` | OpenGL 3.3 core (glad) | windowed, X11 | Debug |
   | `dev-es` | OpenGL ES 3.1 (libGLESv2) | windowed, X11 | Debug |
   | `pi-es` | OpenGL ES 3.1 (libGLESv2) | fullscreen kiosk, Wayland | Release |

   ```sh
   cmake --preset dev-core && cmake --build build/dev-core -j$(nproc)
   ```

   Each preset builds into `build/<preset>/` and copies `shaders/` and `assets/` next to the binary. The presets set two cache variables, `TARGET_PROFILE` (`DEV`|`PI`) and `GL_API` (`CORE`|`ES`), which become the `APP_PROFILE_<X>` and `APP_GL_<X>` compile definitions. `PI` with `CORE` configures with a warning, since the Pi 4 has no OpenGL 3.3 core context.

3. Run it from the repo root:

   ```sh
   ./build/dev-core/calendar
   ```

   It prints the GL version and renderer it got. The window opens on the container's virtual desktop. To view it, open the forwarded port **6080** in a browser (noVNC; the default password is `vscode`).

`pi-es` can't run in the container (there is no Wayland compositor). Install it and copy the result to the Pi:

```sh
cmake --preset pi-es && cmake --build build/pi-es -j$(nproc)
cmake --install build/pi-es          # → install/pi-es/{calendar,shaders/,assets/}
```

The container and Pi OS trixie share the same Debian base, so glibc and libstdc++ match. The binary won't run on Pi OS bookworm.

## Project layout

```
src/app/           application source
src/platform/      windowing and GL context setup (GLFW)
shaders/           GLSL shaders
assets/            textures and other runtime data
lib/glfw/          prebuilt static GLFW 3.5.1, X11 + Wayland (headers, lib, CMake config)
third_party/glad/  glad OpenGL loader (source, compiled as part of the build)
.devcontainer/     dev container definition
CMakeLists.txt     build configuration
CMakePresets.json  dev-core, dev-es and pi-es build presets
```

`build/` and `install/` are generated and git-ignored. clangd reads `build/compile_commands.json` for autocomplete. Point it at the preset you're working on, then restart clangd:

```sh
ln -sfn dev-core/compile_commands.json build/compile_commands.json
```

## Dependencies

### GLFW 3.5.1

`lib/glfw/` holds a static Release build of GLFW with **both the X11 and Wayland backends** (X11 for the dev container's desktop, Wayland for the Raspberry Pi kiosk). It was compiled **inside the dev container for Linux arm64** and won't link on other platforms or architectures (x86, native macOS); the exported CMake target also records absolute container paths for `librt`/`libm`. To rebuild it, or to change versions:

1. Download the GLFW source from https://www.glfw.org/download.html and extract it into the project root (`glfw-3.5.1/` is git-ignored).
2. Inside the dev container:

   ```sh
   rm -rf glfw-3.5.1/build lib/glfw
   cmake -S glfw-3.5.1 -B glfw-3.5.1/build -G Ninja \
     -DCMAKE_BUILD_TYPE=Release \
     -DBUILD_SHARED_LIBS=OFF \
     -DGLFW_BUILD_X11=ON \
     -DGLFW_BUILD_WAYLAND=ON \
     -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
     -DCMAKE_INSTALL_PREFIX="$PWD/lib/glfw"
   cmake --build glfw-3.5.1/build -j"$(nproc)"
   cmake --install glfw-3.5.1/build
   ```

   Set the prefix at configure time (not only with `cmake --install --prefix`), otherwise `glfw3.pc` points at `/usr/local`. Don't add `-march=native`: the library must run on a Raspberry Pi 4 (Cortex-A72).

3. Check that both backends were included:

   ```sh
   ar t lib/glfw/lib/libglfw3.a                               # expect wl_*.o and x11_*.o
   strings lib/glfw/lib/libglfw3.a | grep -E '^3\.5\.1 '      # expect "3.5.1 Wayland X11 GLX Null EGL OSMesa monotonic"
   ```

   If `wl_*` or `x11_*` files are missing, the dev packages are missing: rebuild the container and delete `glfw-3.5.1/build` before configuring again.

The exported `glfw` target brings its own link dependencies (`Threads::Threads`, `rt`, `m`, `dl`), so `target_link_libraries(... glfw)` is enough. GLFW loads X11, Wayland, EGL and GL at runtime with `dlopen`, so none of them are linked. On the Pi kiosk (Wayland), these must be installed at runtime: `libwayland-client0`, `libwayland-cursor0`, `libwayland-egl1`, `libxkbcommon0` and `libEGL`/`libGLESv2`. The X11 libraries are only needed in the dev container.

### glad 0.1.36

`third_party/glad/` was generated at https://glad.dav1d.de with:

- Language: C/C++
- Specification: gl, API gl 3.3
- Profile: core
- Generate a loader: on

To regenerate it (for example for a newer GL version), download the new files and replace `third_party/glad/include` and `third_party/glad/src/glad.c`.

In code, include glad **before** GLFW, and load it after making the context current:

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

glfwMakeContextCurrent(window);
gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
```
