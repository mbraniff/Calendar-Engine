#pragma once
#include "app/GL.h"
#include "app/rendering/text_renderer.hpp"
#include "app/shapes/mask.hpp"
#include "app/shapes/shape.hpp"
#include "app/text/font.hpp"
#include <glm/fwd.hpp>
#include <string_view>
#include <vector>

class Renderer {
public:
    Renderer() { whiteTex_ = uploadMask({255}, 1, 1); }
    void drawText(Font::GlyphSet &set, const std::string_view text, Point pos, const Color &color) {
        GLuint tex = whiteTex_;
        glm::vec4 rect(0, 0, width_, height_);
        if (!masks_.empty()) {
            const Mask& m = masks_.back();
            if (m.tex == 0) return;
            tex = m.tex;
            rect = glm::vec4(m.x0, m.y0, m.w, m.h);
        }
        textRenderer_.setMask(rect, tex);
        float opacity = 1.0f;
        for (float o : opacity_) opacity *= o;
        textRenderer_.draw(set, text, pos, color, opacity);
    }

    void pushClip(const Path& path) {
        const Mask* parent = masks_.empty() ? nullptr : &masks_.back();
        masks_.push_back(Mask::from(path, parent));
    }

    void popClip() {
        glDeleteTextures(1, &masks_.back().tex);
        masks_.pop_back();
    }

    void pushOpacity(float opacity) {
        opacity_.push_back(opacity);
    }

    void popOpacity() {
        opacity_.pop_back();
    }

    void setFrameBuffer(float w, float h) {
        width_ = w; height_ = h;
        textRenderer_.setFrameBuffer(w, h);
    }
private:
    std::vector<float> opacity_;
    std::vector<Mask> masks_;
    GLuint whiteTex_ = 0;
    TextRenderer textRenderer_;
    float width_ = 0, height_ = 0;
};