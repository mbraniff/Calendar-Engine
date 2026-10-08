#pragma once
#include "app/shader.hpp"
#include "app/GL.h"
#include "app/paths.hpp"
#include "app/graphics.hpp"
#include "app/text/text_layout.hpp"
#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>
#include <vector>

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

    void draw(const std::vector<TextEntry>& text, Point pos, Font::GlyphSet& set) {
        shader_.use();
        shader_.setInt("coverage", 0);
        shader_.setInt("clipMask", 1);
        glBindVertexArray(vao_);
        glActiveTexture(GL_TEXTURE0);

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // pos is the top-left of the line; the baseline sits ascender pixels below it.
        for (auto& c : text) {
            float baseline = std::round(pos.y + c.pos.y + set.ascender);
            auto data = glm::vec4(std::round(pos.x + c.pos.x + c.ch->Bearing.x), baseline - c.ch->Bearing.y, c.ch->Size.x, c.ch->Size.y);
            shader_.set4Float("rect", data);
            glBindTexture(GL_TEXTURE_2D, c.ch->TextureID);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

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