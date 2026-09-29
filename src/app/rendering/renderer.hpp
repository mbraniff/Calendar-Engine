#pragma once
#include "app/rendering/text_renderer.hpp"
#include "app/text/font.hpp"
#include <glm/fwd.hpp>
#include <stack>
#include <string_view>

class Renderer {
public:
    void drawText(Font::GlyphSet &set, const std::string_view text, Point pos, const glm::vec3 &color) {
        textRenderer_.draw(set, text, pos, color);
    }

    void pushOpacity(float opacity) {
        opacity_.push(opacity);
    }

    void popOpacity() {
        opacity_.pop();
    }

    void setFrameBuffer(float w, float h) {
        textRenderer_.setFrameBuffer(w, h);
    }
private:
    std::stack<float> opacity_;
    TextRenderer textRenderer_;
};