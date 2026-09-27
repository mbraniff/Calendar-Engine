#include "app/GL.h"
#include <filesystem>
#include <memory>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "platform.h"

namespace {
    class PlatformGlfw : public Platform {
    public:
        ~PlatformGlfw() override  {
            if (win_) glfwDestroyWindow(win_);
            glfwTerminate();
        }

        bool init(const PlatformConfig &cfg) override {
            glfwSetErrorCallback([](int code, const char* msg) {
                std::fprintf(stderr, "GLFW err %d: %s\n", code, msg);
            });

            #ifdef APP_PROFILE_PI
                glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
            #else
                glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
            #endif

            if (!glfwInit()) return false;
            std::printf("GLFW platform %d\n", glfwGetPlatform());

            #ifdef APP_GL_ES
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            #else
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            #endif
            
            GLFWmonitor* mon = nullptr;
            int w = cfg.width, h = cfg.height;
            if (cfg.fullscreen) {
                mon = glfwGetPrimaryMonitor();
                const GLFWvidmode* m = glfwGetVideoMode(mon);
                w = m->width; h = m->height;
            }

            win_ = glfwCreateWindow(w, h, cfg.title, mon, nullptr);
            if (!win_) return false;

            glfwMakeContextCurrent(win_);
            #ifdef APP_GL_CORE
                if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
                    std::fprintf(stderr, "Failed to load GL functions\n");
                    return false;
                }
            #endif
            std::printf("%s | %s\n", glGetString(GL_VERSION), glGetString(GL_RENDERER));

            glfwSwapInterval(1);
            if (cfg.hideCursor) glfwSetInputMode(win_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            glfwSetWindowUserPointer(win_, this);
            glfwSetMouseButtonCallback(win_, [](GLFWwindow* w, int btn, int act, int) {
                auto* self = static_cast<PlatformGlfw*>(glfwGetWindowUserPointer(w));
                if (btn != GLFW_MOUSE_BUTTON_LEFT || !self->onPointer) return;
                double x, y; glfwGetCursorPos(w, &x, &y);
                self->onPointer((float)x, (float)y, act == GLFW_PRESS);
            });
            glfwSetKeyCallback(win_, [](GLFWwindow* w, int key, int, int act, int) {
                auto* self = static_cast<PlatformGlfw*>(glfwGetWindowUserPointer(w));
                if (act == GLFW_PRESS && self->onKey) self->onKey(key);
            });
            return true;
        }

        bool shouldClose() const override { return glfwWindowShouldClose(win_); }
        void pollEvents() override { glfwPollEvents(); }
        void swapBuffers() override { glfwSwapBuffers(win_); }
        double time() const override { return glfwGetTime(); }
        void framebufferSize(int &w, int &h) const override {
            glfwGetFramebufferSize(win_, &w, &h);
        }
        bool isKeyDown(int key) override { return glfwGetKey(win_, key) == GLFW_PRESS; }
        void requestClose() override { glfwSetWindowShouldClose(win_, GLFW_TRUE); }

    private:
        GLFWwindow* win_ = nullptr;
    };
}

std::unique_ptr<Platform> createPlatform() {
    return std::make_unique<PlatformGlfw>();
}

std::filesystem::path exeDir() {
    return std::filesystem::read_symlink("/proc/self/exe").parent_path();
}