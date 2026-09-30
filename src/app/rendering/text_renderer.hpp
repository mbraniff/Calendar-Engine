#pragma once
#include "app/text/font.hpp"
#include "app/shader.hpp"
#include "app/GL.h"
#include "app/paths.hpp"
#include "app/graphics.hpp"
#include <cmath>
#include <string_view>

class TextRenderer {
public:
    TextRenderer() : shader_(paths::shader("glyph.vert").c_str(), paths::shader("glyph.frag").c_str()) {
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

    ~TextRenderer() {
        glDeleteBuffers(1, &vbo_);
        glDeleteVertexArrays(1, &vao_);
    }

    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;

    void draw(Font::GlyphSet& set, std::string_view text, Point pos, Color color, float opacity) {
        shader_.use();
        shader_.setFloat("opacity", opacity);
        glUniform3f(glGetUniformLocation(shader_.ID, "textColor"), color.r, color.g, color.b);
        glBindVertexArray(vao_);
        glActiveTexture(GL_TEXTURE0);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // pos is the top-left of the line; the baseline sits ascender pixels below it.
        float x = pos.x;
        float baseline = std::round(pos.y + set.ascender);
        for (auto& c : text) {
            auto& ch = set.chars[c];
            auto data = glm::vec4(x + ch.Bearing.x, baseline - ch.Bearing.y, ch.Size.x, ch.Size.y);
            shader_.set4Float("rect", data);
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            x += ch.Advance >> 6;
        }
        glBindVertexArray(0);
        glDisable(GL_BLEND);
    }

    void setFrameBuffer(float w, float h) {
        // Top-left is (0, 0) and y grows down.
        glm::mat4 textProjection = glm::ortho(0.0f, (float)w, (float)h, 0.0f);
        shader_.use();
        shader_.setMat4("projection", textProjection);
    }

private:
    unsigned int vao_ = 0, vbo_ = 0;
    Shader shader_;
};