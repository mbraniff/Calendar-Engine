# calendar-engine

A C++20 / OpenGL 3.3 core project, built with GLFW and glad. Currently working through [LearnOpenGL](https://learnopengl.com).

## Requirements

- Docker
- VS Code with the Dev Containers extension

Everything else (compiler, CMake, Ninja, X11, Wayland, EGL/GLES and Mesa dev packages) is installed in the dev container. See `.devcontainer/Dockerfile`.

## Getting started

1. Open the folder in VS Code and run **Dev Containers: Reopen in Container**.
2. Build inside the container:

   ```sh
   cmake -S . -B build -G Ninja
   cmake --build build
   ```

3. Run it:

   ```sh
   ./build/calendar
   ```

The window opens on the container's virtual desktop. To view it, open the forwarded port **6080** in a browser (noVNC; the default password is `vscode`).

## Project layout

```
src/             application source
lib/glfw/        prebuilt static GLFW 3.5.1, X11 + Wayland (headers, lib, CMake config)
lib/glad/        glad OpenGL loader (source, compiled as part of the build)
.devcontainer/   dev container definition
CMakeLists.txt   build configuration
```

`build/` is generated and git-ignored. clangd reads `build/compile_commands.json` for autocomplete, so re-run the CMake configure step if autocomplete stops finding headers.

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

`lib/glad/` was generated at https://glad.dav1d.de with:

- Language: C/C++
- Specification: gl, API gl 3.3
- Profile: core
- Generate a loader: on

To regenerate it (for example for a newer GL version), download the new files and replace `lib/glad/include` and `lib/glad/src/glad.c`.

In code, include glad **before** GLFW, and load it after making the context current:

```cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>

glfwMakeContextCurrent(window);
gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
```
