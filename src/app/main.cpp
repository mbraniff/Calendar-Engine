#include <algorithm>
#include <cmath>
#include <iostream>
#include "GL.h"
#include <GLFW/glfw3.h>
#include <string>
#include "shader.hpp"
#include "stb_image.h"
#include "platform/platform.h"

const char *VERTEX_FILE_PATH = "./shaders/shader.vert";
const char *FRAGMENT_FILE_PATH = "./shaders/shader.frag";
float opacity = 0.2f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(int key) {
    std::cout << "Hit key: " << key << std::endl;
    if(key == GLFW_KEY_ESCAPE) {
        // glfwSetWindowShouldClose(window, true);
    }
    if(key ==  GLFW_KEY_UP || key == GLFW_KEY_KP_8) {
        opacity = std::min(opacity + 0.1f, 1.0f);
    }
    if(key == GLFW_KEY_DOWN || key == GLFW_KEY_KP_2) {
        opacity = std::max(opacity - 0.1f, 0.0f);
    }
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

int main() {
    #ifdef APP_PROFILE_DEV
        PlatformConfig cfg{800, 600, "app", false, false};
    #else
        PlatformConfig cfg{1280, 800, "app", true, true};
    #endif

    auto platform = createPlatform();
    if (!platform->init(cfg)) return 1;
    platform->onKey = [&](int key) {
        processInput(key);
    };

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

    Shader program(VERTEX_FILE_PATH, FRAGMENT_FILE_PATH);
    unsigned int container = loadTexture("./assets/container.jpg");
    unsigned int smile = loadTexture("./assets/awesomeface.png", true);

    program.use();
    glUniform1i(glGetUniformLocation(program.ID, "texture1"), 0);
    program.setInt("texture2", 1);

    unsigned int rect2 = generateRectangleObject2();

    while (!platform->shouldClose()) {

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        program.use();
        program.setFloat("opacity", opacity);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, container);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, smile);

        glBindVertexArray(rectangle);
        // glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        platform->swapBuffers();
        platform->pollEvents();
    }
    platform = nullptr;
    return 0;
}

