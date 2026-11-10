#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "calendar/playground_view_controller.hpp"
#include "ui/GL.h"
#include <GLFW/glfw3.h>
#include <memory>
#include "calendar/views/fps_view.hpp"
#include "ui/animation/animator.hpp"
#include "ui/graphics.hpp"
#include "ui/pointer_dispatch.hpp"
#include "ui/rendering/renderer.hpp"
#include "platform/platform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <freetype/freetype.h>
#include <unistd.h>
#include "calendar/content_view.hpp"
#include <thread>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(Platform* platform, float deltaTime) {
    float cameraSpeed = 2.5f * deltaTime;
    if(platform->isKeyDown(GLFW_KEY_ESCAPE)) {
        platform->requestClose();
    }
}

void processPointer(PointerDispatch& dispatch, PointerEvent& e) {
    switch (e.action) {
    case PointerAction::Down:
        dispatch.pointerDown(e.channel, e.p, e.time);
        break;
    case PointerAction::Move:
        dispatch.pointerMove(e.channel, e.p, e.time);
        break;
    case PointerAction::Up:
        dispatch.pointerUp(e.channel, e.p, e.time);
        break;
    }
}

int main() {
    #ifdef APP_PROFILE_DEV
        PlatformConfig cfg{800, 600, "app", false, false};
    #else
        PlatformConfig cfg{1280, 800, "app", true, true};
    #endif

    auto platform = createPlatform();
    if (!platform->init(cfg)) return 1;
    {
        int w, h;
        platform->framebufferSize(w, h);

        Renderer renderer;
        auto mainView = std::make_shared<View>();
        renderer.setFrameBuffer(w, h);
        platform->onFramebufferResize = [&renderer, &w, &h](int wi, int he) {
            glViewport(0, 0, wi, he);
            renderer.setFrameBuffer(wi, he);
            w = wi; h = he;
        };
        PointerDispatch dispatch = PointerDispatch(mainView);
        platform->onPointer = [&](PointerEvent e) {
            processPointer(dispatch, e);
        };

        // ContentViewViewController content;
        PlaygroundViewController content;
        mainView->addSubview(content.view());
        mainView->addSubview(std::make_shared<FPSView>());

        int maxFrames = 60;
        using clock = std::chrono::steady_clock;
        const auto frameBudget = std::chrono::microseconds(1'000'000 / maxFrames);
        auto nextFrame = clock::now();
        bool frameLimit = false;
        float lastFrame = 0.0f;
        float deltaTime = 0.0f;

        while (!platform->shouldClose()) {
            float currentFrame = platform->time();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            processInput(platform.get(), deltaTime);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ViewController::tickAll(deltaTime);

            Animator::shared().tick(deltaTime);
            mainView->layout({{0, 0 }, (float)w, (float)h});
            mainView->render(renderer);

            platform->swapBuffers();
            platform->pollEvents();

            if (frameLimit) {
                nextFrame += frameBudget;
                std::this_thread::sleep_until(nextFrame);
                if (clock::now() > nextFrame + frameBudget) nextFrame = clock::now();
            }
        }
    }
    platform = nullptr;
    return 0;
}

