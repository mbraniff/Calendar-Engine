#pragma once
#include "ui/shapes/mask.hpp"
#include "ui/shader.hpp"
#include "ui/GL.h"
#include "ui/paths.hpp"
#include "ui/graphics.hpp"
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>

class ShapeRenderer {
public:
    ShapeRenderer() : shader_(paths::shader("glyph.vert").c_str(), paths::shader("glyph.frag").c_str()) {
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

    ~ShapeRenderer() {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }

    ShapeRenderer(const ShapeRenderer&) = delete;
    ShapeRenderer& operator=(const ShapeRenderer&) = delete;

    void draw(Mask& coverage, Point offset) {
        shader_.use();
        shader_.setInt("coverage", 0);
        shader_.setInt("clipMask", 1);
        glBindVertexArray(vao_);
        glActiveTexture(GL_TEXTURE0);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader_.set4Float("rect", glm::vec4(coverage.x0 + offset.x, coverage.y0 + offset.y, coverage.w, coverage.h));
        glBindTexture(GL_TEXTURE_2D, coverage.tex);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glBindVertexArray(0);
        glDisable(GL_BLEND);
    }

    void setFrameBuffer(float w, float h) {
        // Top-left is (0, 0) and y grows down.
        glm::mat4 textProjection = glm::ortho(0.0f, (float)w, (float)h, 0.0f);
        shader_.use();
        shader_.setMat4("projection", textProjection);
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
    
    void setColor(const Color& color) {
        shader_.use();
        shader_.set4Float("color", glm::vec4(color.r, color.g, color.b, color.a));
    }

private:
    unsigned int vao_ = 0, vbo_ = 0;
    Shader shader_;
};