#include <cmath>
#include <cstdio>
#include <cstdlib>
#include "GL.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include "app/button/button.hpp"
#include "app/containers/hstack.hpp"
#include "app/containers/vstack.hpp"
#include "app/gestures/gesture.hpp"
#include "app/gestures/tap_gesture.hpp"
#include "app/graphics.hpp"
#include "app/pointer_dispatch.hpp"
#include "app/rendering/renderer.hpp"
#include "app/shapes/shape.hpp"
#include "app/shapes/shape_view.hpp"
#include "app/text/text.hpp"
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

void processPointer(PointerDispatch& dispatch, int channel, float x, float y, PointerAction action) {
    switch (action) {
    case PointerAction::Down:
        dispatch.pointerDown(channel, {x, y});
        break;
    case PointerAction::Move:
        dispatch.pointerMove(channel, {x, y});
        break;
    case PointerAction::Up:
        dispatch.pointerUp(channel, {x, y});
        break;
    }
}

// std::vector<PlacedGlyph> layoutText(std::string_view text, glm::vec2 origin, float maxWidth, float lineHeight) {
//     std::vector<PlacedGlyph> out;
//     float x = origin.x, y = origin.y;
//     size_t lineStart = 0;
//     size_t wordStart = 0;
//     float wordStartX = x;

//     for (char c : text) {
//         if (c == '\n') {
//             x = origin.x; y -= lineHeight;
//             lineStart = wordStart = out.size();
//             wordStartX = x;
//             continue;
//         }
//         const Character &ch = Characters[c];
//         float advance = ch.Advance >> 6;

//         if (c == ' ') {
//             x += advance;
//             wordStart = out.size();
//             wordStartX = x;
//             continue;
//         }

//         if (x + advance - origin.x > maxWidth && wordStart > lineStart) {
//             float shift = wordStartX - origin.x;
//             for (size_t i = wordStart; i < out.size(); ++i) {
//                 for (auto& v : out[i].quad) { v.pos.x -= shift; v.pos.y -= lineHeight; }
//             }
//             x -= shift; y -= lineHeight;
//             lineStart = wordStart;
//             wordStartX = origin.x;
//         }

//         glm::vec2 pos{std::round(x + ch.Bearing.x), std::round(y - (ch.Size.y - ch.Bearing.y))};
//         out.push_back({ch.TextureID, buildQuad(pos, ch.Size)});
//         x += advance;
//     }
//     return out;
// }

// void drawText(const std::vector<PlacedGlyph> glyphs, unsigned int VAO, unsigned int VBO) {
//     std::vector<GlyphVertex> verts;
//     verts.reserve(glyphs.size() * 6);
//     for (auto& g : glyphs) verts.insert(verts.end(), g.quad.begin(), g.quad.end());

//     glBindVertexArray(VAO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GlyphVertex), verts.data(), GL_DYNAMIC_DRAW);
//     for (size_t i = 0; i < glyphs.size(); ++i) {
//         glBindTexture(GL_TEXTURE_2D, glyphs[i].texture);
//         glDrawArrays(GL_TRIANGLES, i*6, 6);
//     }
//     glBindVertexArray(0);
// }

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
        platform->onPointer = [&](int channel, float x, float y, PointerAction action) {
            processPointer(dispatch, channel, x, y, action);
        };
        text->alignment = Alignment::topCenter();
        text->padding(12, Padding::Horizontal);
        text->padding(8, Padding::Vertical);
        auto subText = std::make_shared<Text>("This is some text under the original text");
        subText->alignment = Alignment::centerLeading();
        subText->offset.x = -100.0f;
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

        std::shared_ptr<Button> b = std::make_shared<Button>(text, [&counter, weak = std::weak_ptr<Text>(counterView)]() {
            counter += 1;
            if (auto t = weak.lock()) {
                t->text = std::to_string(counter);
            }
        });
        b->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color(0.8f, 0.0f, 0.8f));
        b->clipShape = std::make_unique<Capsule>();

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
        subText->background = std::make_shared<ShapeView>(std::make_shared<Rectangle>(), Color::blue());

        auto vstack = std::make_shared<VStack>();
        auto hstack = std::make_shared<HStack>(0);
        auto frame = vstack->frame({.maxHeight = infinity});
        vstack->padding(8);
        vstack->addSubview(b);
        vstack->addSubview(subText);
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
        while (!platform->shouldClose()) {
            float currentFrame = platform->time();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            processInput(platform.get(), deltaTime);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float speed = 25.0f;
            subText->offset.x += dirx * speed * deltaTime;
            subText->offset.y += diry * speed * deltaTime;
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

