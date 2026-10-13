#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "GL.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include "app/animation/animator.hpp"
#include "app/animation/motion.hpp"
#include "app/button/button.hpp"
#include "app/containers/frame.hpp"
#include "app/containers/hstack.hpp"
#include "app/containers/vstack.hpp"
#include "app/gestures/drag_gesture.hpp"
#include "app/gestures/gesture.hpp"
#include "app/gestures/tap_gesture.hpp"
#include "app/graphics.hpp"
#include "app/pointer_dispatch.hpp"
#include "app/rendering/renderer.hpp"
#include "app/shapes/shape.hpp"
#include "app/shapes/shape_view.hpp"
#include "app/text/text.hpp"
#include "app//text/text_layout.hpp"
#include "stb_image.h"
#include "platform/platform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <freetype/freetype.h>
#include <unistd.h>
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
        // platform->onKey = [&, program](int key) {
        //     processInput(key, program.ID);
        // };

        int w, h;
        platform->framebufferSize(w, h);

        Renderer renderer;
        auto mainView = std::make_shared<View>();
        auto text = std::make_shared<Text>("Howdy yall this is some text that will render bad");
        text->fontSize = 14; 
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
        text->alignment = Alignment::topCenter();
        text->padding(12, Padding::Horizontal);
        text->padding(8, Padding::Vertical);
        auto subText = std::make_shared<Text>("This is some text under the original text");
        subText->alignment = Alignment::centerLeading();
        subText->offset.update([](Point& p) {
            p.x = -100.0f; 
        });
        subText->color = Color::red();
        subText->fontWeight = Font::Weight::Heavy;
        int counter = 0;
        auto counterView = std::make_shared<Text>("0");
        counterView->alignment = Alignment::bottomTrailing();
        subText->addGesture(std::make_shared<TapGestureRecognizer>([&counter, weak = std::weak_ptr<Text>(counterView)](std::shared_ptr<GestureRecognizer> sender){
            counter += 1;
            if (auto t = weak.lock()) {
                t->text = std::to_string(counter);
            }
        }));

        std::shared_ptr<Text> bigTextLabel = std::make_shared<Text>("Text that is too large for its frame, something that is biggy: like way too biggy");
        bigTextLabel->color = Color::white();
        bigTextLabel->setTextAlignment(TextAlignment::Right);
        auto bigText = bigTextLabel->frame({.maxWidth=100, .maxHeight=100});
        bigText->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color::blue());
        auto btg = std::make_shared<DragGestureRecognizer>();
        btg->onChange([weak = std::weak_ptr(bigText)](DragGestureRecognizer::DragEvent event) {
            auto view = weak.lock();
            ((Frame*)view.get())->options.maxWidth.value() += event.translation.width;
            ((Frame*)view.get())->options.maxHeight.value() += event.translation.height;
        });
        bigText->addGesture(btg);

        std::shared_ptr<Button> b = std::make_shared<Button>(text, [&counter, weak = std::weak_ptr<Text>(counterView)]() {
            counter += 1;
            if (auto t = weak.lock()) {
                t->text = std::to_string(counter);
            }
        });
        b->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color(0.8f, 0.0f, 0.8f));
        b->clipShape = std::make_unique<Capsule>();
        std::shared_ptr<DragGestureRecognizer> dragGesture = std::make_shared<DragGestureRecognizer>();
        dragGesture->onChange([weak = std::weak_ptr<View>(b)](DragGestureRecognizer::DragEvent event){
            auto view = weak.lock();
            Point offset = view->offset;
            view->offset = { offset.x + event.translation.width, offset.y + event.translation.height };
        }).onEnd([weak = std::weak_ptr<View>(b), &w, &h](DragGestureRecognizer::DragEvent event) {
            auto view = weak.lock();
            if (!view) return;

            constexpr float d = 0.99f;
            auto project = [](float v) { return (v / 1000.0f) * d / (1.0f - d); };

            Point anchor = view->anchor();
            Point offset = view->offset;   // current, including this poll's moves
            Size s = view->size();

            float x = std::clamp(anchor.x + offset.x + event.translation.width + project(event.velocity.width),
                                0.0f, (float)w - s.width);
            float y = std::clamp(anchor.y + offset.y + event.translation.height + project(event.velocity.height),
                                0.0f, (float)h - s.height);

            animate(Animation::spring(), [&] {
                view->offset.setVelocity({ event.velocity.width, event.velocity.height });
                view->offset = { x - anchor.x, y - anchor.y };
            });
        });
        b->addGesture(dragGesture);

        // Debug FPS counter, averaged over half a second so it's readable.
        auto fps = std::make_shared<Text>("FPS --");
        fps->alignment = Alignment::topTrailing();
        fps->offset = { -8.0f, 8.0f };
        fps->color = Color::green();
        int fpsFrames = 0;
        float fpsElapsed = 0.0f;

        int maxFrames = 60;
        using clock = std::chrono::steady_clock;
        const auto frameBudget = std::chrono::microseconds(1'000'000 / maxFrames);
        auto nextFrame = clock::now();
        bool frameLimit = false;
        float lastFrame = 0.0f;
        float deltaTime = 0.0f;
        subText->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color::blue());

        auto vstack = std::make_shared<VStack>();
        auto hstack = std::make_shared<HStack>(0);
        auto frame = vstack->frame({.maxHeight = infinity});
        auto swapAlignment = std::make_shared<Button>(std::make_shared<Text>("Swap"), [weak = std::weak_ptr(bigTextLabel)]{
            animate(Animation::spring(), [&]{
                auto view = weak.lock();
                if (!view) return;
                auto text = static_cast<Text*>(view.get());
                switch (text->getTextAlignment()) {
                case TextAlignment::Left:
                    text->setTextAlignment(TextAlignment::Centered);
                    break;
                case TextAlignment::Centered:
                    text->setTextAlignment(TextAlignment::Right);
                    break;
                case TextAlignment::Right:
                    text->setTextAlignment(TextAlignment::Left);
                    break;
                }
            });
        });
        vstack->padding(8);
        vstack->addSubview(b);
        vstack->addSubview(subText);
        vstack->addSubview(bigText);
        vstack->addSubview(swapAlignment);

        hstack->clipShape = std::make_unique<Rectangle>();
        // auto bg = std::make_shared<ShapeView>(std::make_shared<Rectangle>(), Color::red());
        // frame->background = bg;
        hstack->addSubview(std::make_shared<ShapeView>(std::make_shared<Circle>(), Color::blue())->frame({.width = 100}));
        hstack->addSubview(std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(25), Color::green())->frame({.width = 200, .height = 200}));
        hstack->padding(8);
        mainView->addSubview(hstack);
        mainView->addSubview(frame);
        mainView->addSubview(fps);
        mainView->addSubview(counterView);
        subText->padding(8);
        float dirx = 1, diry = 1;

        animate(Animation::autoreverse(Animation::linear(1.0f), 0), [&] {
            subText->background->opacity = 0.2f;
        });

        while (!platform->shouldClose()) {
            float currentFrame = platform->time();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            processInput(platform.get(), deltaTime);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float speed = 25.0f;
            subText->offset.update([&](Point& p) {
                p.x += dirx * speed * deltaTime;
                p.y += diry * speed * deltaTime;
            });
            Rect f = subText->frame();
            if (f.origin.y + f.height >= h) diry = -1;
            if (f.origin.y <= 0)            diry =  1;
            if (f.origin.x + f.width >= w)  dirx = -1;
            if (f.origin.x <= 0)            dirx =  1;

            fpsFrames++;
            fpsElapsed += deltaTime;
            if (fpsElapsed >= 0.5f) {
                fps->text = "FPS " + std::to_string((int)std::round(fpsFrames / fpsElapsed));
                fpsFrames = 0;
                fpsElapsed = 0.0f;
            }

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

