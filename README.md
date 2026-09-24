# calendar-engine

A C++20 / OpenGL 3.3 core project, built with GLFW and glad. Currently working through [LearnOpenGL](https://learnopengl.com).

## Requirements

- Docker
- VS Code with the Dev Containers extension

Everything else (compiler, CMake, Ninja, X11 and Mesa dev packages) is installed in the dev container. See `.devcontainer/Dockerfile`.

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
lib/glfw/        prebuilt GLFW 3.5.1 (headers, static lib, CMake config)
lib/glad/        glad OpenGL loader (source, compiled as part of the build)
.devcontainer/   dev container definition
CMakeLists.txt   build configuration
```

`build/` is generated and git-ignored. clangd reads `build/compile_commands.json` for autocomplete, so re-run the CMake configure step if autocomplete stops finding headers.

## Dependencies

### GLFW 3.5.1

`lib/glfw/` holds a static build of GLFW, compiled **inside the dev container for Linux arm64** with the X11 backend. It won't link on other platforms or architectures (x86, native macOS). To rebuild it there, or to change versions:

1. Download the GLFW source from https://www.glfw.org/download.html and extract it into the project root (`glfw-3.5.1/` is git-ignored).
2. Inside the dev container:

   ```sh
   rm -rf glfw-3.5.1/build
   cmake -S glfw-3.5.1 -B glfw-3.5.1/build -DGLFW_BUILD_WAYLAND=OFF -DGLFW_BUILD_X11=ON
   cmake --build glfw-3.5.1/build
   cmake --install glfw-3.5.1/build --prefix ./lib/glfw
   ```

3. Check that the X11 backend was included. The output should list `x11_*` and `glx_context` object files:

   ```sh
   ar t lib/glfw/lib/libglfw3.a
   ```

   If it only shows `null_*` files, the X11 dev packages are missing: rebuild the container and delete `glfw-3.5.1/build` before configuring again.

GLFW loads X11 and libGL at runtime, so neither needs to be linked explicitly.

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
