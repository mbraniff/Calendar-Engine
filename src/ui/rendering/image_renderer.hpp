#pragma once
#include "ui/shader.hpp"
#include "ui/GL.h"
#include "ui/paths.hpp"
#include "ui/graphics.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

class ImageRenderer {
public:
    ImageRenderer() : shader_(paths::shader("image.vert").c_str(), paths::shader("image.frag").c_str()) {
        float corners[] = { 0,0, 1,0, 0,1, 1,1 };
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(corners), corners, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glBindVertexArray(0);
    }

    ~ImageRenderer() {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }

    ImageRenderer(const ImageRenderer&) = delete;
    ImageRenderer& operator=(const ImageRenderer&) = delete;

    // Draws uv's part of image stretched over rect, which is in screen space.
    void draw(GLuint image, const Rect& rect, const glm::vec4& uv, const Color& color, float templateMode) {
        shader_.use();
        shader_.setInt("image", 0);
        shader_.setInt("clipMask", 1);
        shader_.set4Float("rect", glm::vec4(rect.origin.x, rect.origin.y, rect.width, rect.height));
        shader_.set4Float("uvRect", uv);
        shader_.set4Float("color", glm::vec4(color.r, color.g, color.b, color.a));
        shader_.setFloat("templateMode", templateMode);
        glBindVertexArray(vao_);
        // setMask left unit 1 active; binding there would replace the clip with the image.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);
        glDisable(GL_BLEND);
    }

    void setFrameBuffer(float w, float h) {
        // Top-left is (0, 0) and y grows down.
        glm::mat4 projection = glm::ortho(0.0f, (float)w, (float)h, 0.0f);
        shader_.use();
        shader_.setMat4("projection", projection);
        shader_.setFloat("viewportHeight", h);
    }

    void setMask(const glm::vec4& pos, GLuint tex) {
        shader_.use();
        shader_.set4Float("maskRect", pos);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    void setOpacity(const float opacity) {
        shader_.use();
        shader_.setFloat("opacity", opacity);
    }

private:
    unsigned int vao_ = 0, vbo_ = 0;
    Shader shader_;
};
