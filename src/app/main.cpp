#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include "GL.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include "app/containers/vstack.hpp"
#include "app/graphics.hpp"
#include "app/rendering/renderer.hpp"
#include "app/shapes/shape.hpp"
#include "app/shapes/shape_view.hpp"
#include "app/text/text.hpp"
#include "shader.hpp"
#include "stb_image.h"
#include "platform/platform.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "app/paths.hpp"
#include <freetype/freetype.h>
#include <map>
#include <set>
#include <string_view>
#include <unistd.h>
#include <vector>
#include <thread>

static std::set<char> whitespace = { '\n', ' ', '\t', '\r' };

float opacity = 0.2f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void setView(unsigned int program) {
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    int viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processInput(Platform* platform, unsigned int program, float deltaTime) {
    float cameraSpeed = 2.5f * deltaTime;
    if(platform->isKeyDown(GLFW_KEY_ESCAPE)) {
        platform->requestClose();
    }
    if(platform->isKeyDown(GLFW_KEY_W)) {
        cameraPos += cameraSpeed * cameraFront;
    }
    if(platform->isKeyDown(GLFW_KEY_S)) {
        cameraPos -= cameraSpeed * cameraFront;
    }
    if(platform->isKeyDown(GLFW_KEY_A)) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if(platform->isKeyDown(GLFW_KEY_D)) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    setView(program);
}

unsigned int loadTexture(const std::string &path, bool flip = false) {
    int format = path.find(".png") != std::string::npos ? GL_RGBA : GL_RGB;
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(flip);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    return texture;
}

unsigned int generateRectangleObject2() {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    float vertices[] = {
        // positions                    // colors                        // texture coords
        0.5f, 0.5f, 0.0f,   1.0f, 0.0f, 0.0f,    2.0f, 2.0f, // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 2.0f // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return VAO;
}

unsigned int generateCube() {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    float vertices[] = {
        // position              // color          // uv
        // Front (z = +0.5)
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        // Back (z = -0.5)
        0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        // Right (x = +0.5)
        0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        // Left (x = -0.5)
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        // Top (y = +0.5)
        -0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.5f,  0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        // Bottom (y = -0.5)
        -0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,     0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    };

    unsigned int indices[36];
    for (unsigned int face = 0; face < 6; ++face) {
        unsigned int b = face * 4;
        unsigned int quad[] = { b, b + 1, b + 2,   b + 2, b + 3, b };
        std::copy(std::begin(quad), std::end(quad), indices + face * 6);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return VAO;
}

unsigned int generateRectangleObject() {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    float vertices[] = {
        // positions                    // colors                        // texture coords
        0.5f, 0.5f, 0.0f,   1.0f, 0.0f, 0.0f,    1.0f, 1.0f, // top right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return VAO;
}

unsigned int generateTriangleArray(float vertices[], size_t size) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    return VAO;
}

unsigned int generateTriangleObject() {
    float vertices[] = {
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    return generateTriangleArray(vertices, sizeof(vertices));
}

struct GlyphVertex { glm::vec2 pos; glm::vec2 uv; };
using GlyphQuad = std::array<GlyphVertex, 6>;
GlyphQuad buildQuad(glm::vec2 p, glm::vec2 s) {
    return {{
        {{p.x,       p.y + s.y}, {0, 0}},
          {{p.x,       p.y      }, {0, 1}},
          {{p.x + s.x, p.y      }, {1, 1}},
          {{p.x,       p.y + s.y}, {0, 0}},
          {{p.x + s.x, p.y      }, {1, 1}},
          {{p.x + s.x, p.y + s.y}, {1, 0}},
    }};
}

struct PlacedGlyph {
    unsigned int texture;
    GlyphQuad quad;
};

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
        unsigned int triangle = generateTriangleObject();
        unsigned int rectangle = generateRectangleObject();
        float triangleOneVertices[] = {
            -1.0f, -0.5f, 0.0f,
            0.0f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f,
        };
        float triangleTwoVertices[] = {
            1.0f, -0.5f, 0.0f,
            0.0f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f
        };
        unsigned int triangleOne = generateTriangleArray(triangleOneVertices, sizeof(triangleOneVertices));
        unsigned int triangleTwo = generateTriangleArray(triangleTwoVertices, sizeof(triangleTwoVertices));

        Shader program(paths::shader("shader.vert").c_str(), paths::shader("shader.frag").c_str());
        unsigned int container = loadTexture(paths::asset("container.jpg"));
        unsigned int smile = loadTexture(paths::asset("awesomeface.png"), true);

        program.use();
        glUniform1i(glGetUniformLocation(program.ID, "texture1"), 0);
        program.setInt("texture2", 1);

        // platform->onKey = [&, program](int key) {
        //     processInput(key, program.ID);
        // };

        unsigned int rect2 = generateRectangleObject2();
        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 projection;

        int w, h;
        platform->framebufferSize(w, h);
        projection = glm::perspective(glm::radians(45.0f), (float)w/(float)h, 0.1f, 100.0f);

        int modelLoc = glGetUniformLocation(program.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        setView(program.ID);
        int projLoc = glGetUniformLocation(program.ID, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        unsigned int cube = generateCube();

        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)
        };

        Renderer renderer;
        View mainView;
        auto text = std::make_shared<Text>("Howdy yall this is some text that will render bad");
        renderer.setFrameBuffer(w, h);
        platform->onFramebufferResize = [&renderer, &w, &h](int wi, int he) {
            glViewport(0, 0, wi, he);
            renderer.setFrameBuffer(wi, he);
            w = wi; h = he;
        };
        text->alignment = Alignment::topCenter();
        text->clipShape = std::make_unique<Capsule>();
        text->background = std::make_shared<ShapeView>(std::make_shared<RoundedRectangle>(5.0f), Color::green());
        auto subText = std::make_shared<Text>("This is some text under the original text longer string of text so it cuts off past the body");
        subText->alignment = Alignment::centerLeading();
        subText->offset.x = -100.0f;
        subText->color = Color::red();
        subText->fontWeight = Font::Weight::Heavy;

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
        auto frame = vstack->frame({.maxWidth = F_INFINITY, .maxHeight = F_INFINITY});
        vstack->padding(8);
        vstack->addSubview(text);
        vstack->addSubview(subText);
        vstack->clipShape = std::make_unique<Rectangle>();
        auto bg = std::make_shared<ShapeView>(std::make_shared<Rectangle>(), Color::red());
        frame->background = bg;
        mainView.addSubview(frame);
        mainView.addSubview(fps);
        subText->padding(8);
        while (!platform->shouldClose()) {
            float currentFrame = platform->time();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            processInput(platform.get(), program.ID, deltaTime);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // program.use();
            // program.setFloat("opacity", opacity);

            // glEnable(GL_DEPTH_TEST);
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, container);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            // glActiveTexture(GL_TEXTURE1);
            // glBindTexture(GL_TEXTURE_2D, smile);

            // glBindVertexArray(cube);
            // for(unsigned int i = 0; i < 10; i++) {
            //     glm::mat4 model = glm::mat4(1.0f);
            //     model = glm::translate(model, cubePositions[i]);
            //     float angle = 20.0f * i;
            //     model = glm::rotate(model, i%3==0 ? (float)platform->time() : 1 * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            //     program.setMat4("model", model);
            //     glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            // }
            // glDrawArrays(GL_TRIANGLES, 0, 3);

            // glBindVertexArray(0);

            subText->offset.x += 25.0f * deltaTime;

            fpsFrames++;
            fpsElapsed += deltaTime;
            if (fpsElapsed >= 0.5f) {
                fps->text = "FPS " + std::to_string((int)std::round(fpsFrames / fpsElapsed));
                fpsFrames = 0;
                fpsElapsed = 0.0f;
            }

            mainView.layout({{0, 0 }, (float)w, (float)h});
            mainView.render(renderer);

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

